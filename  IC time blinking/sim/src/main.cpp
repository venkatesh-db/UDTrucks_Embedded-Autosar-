#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

struct TimeValue {
    int hour{0};
    int minute{0};
    int second{0};
    bool valid{false};
    Clock::time_point lastUpdate{}; // monotonic timestamp
};

// Simple exclusive area abstraction (SchM) using std::mutex
struct ExclusiveArea {
    std::mutex mtx;
    void enter() { mtx.lock(); }
    void exit() { mtx.unlock(); }
};

// Double-buffered time cache for robust pattern
class TimeCache {
public:
    TimeCache() { buffers_[0].valid = false; buffers_[1].valid = false; }

    void write(const TimeValue& tv) {
        // swap-write: write into the non-active buffer then flip index
        int next = 1 - active_.load(std::memory_order_relaxed);
        buffers_[next] = tv;
        active_.store(next, std::memory_order_release);
    }

    TimeValue readSnapshot() const {
        int idx = active_.load(std::memory_order_acquire);
        return buffers_[idx]; // copy snapshot
    }

private:
    TimeValue buffers_[2];
    std::atomic<int> active_{0};
};

struct Stats {
    std::atomic<uint64_t> produced{0};
    std::atomic<uint64_t> consumed{0};
    std::atomic<uint64_t> blanks{0};
    std::atomic<uint64_t> invalidTransitions{0};
};

struct StressConfig {
    // periods
    std::chrono::milliseconds prodPeriod{100};   // time producer period (100 ms)
    std::chrono::milliseconds dispPeriod{50};    // display consumer period (50 ms)
    std::chrono::milliseconds timeout{200};      // tighter timeout to provoke naïve blinking
    std::chrono::milliseconds grace{250};        // longer grace to let robust path hold value

    // stress knobs
    double isrBusyProbability{0.30};             // higher chance of ISR-like bursts
    std::chrono::milliseconds isrBusyMax{220};   // larger burst length

    double busDropProbability{0.20};             // more frequent late/dropped sync
    std::chrono::milliseconds busLateMax{300};   // greater lateness window
};

struct SimContext {
    StressConfig cfg;
    std::atomic<bool> stop{false};
    Stats statsNaive;   // for naive path
    Stats statsRobust;  // for robust path
};

// Utility: format time as HH:MM:SS
static std::string fmt(const TimeValue& tv) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << tv.hour << ":"
        << std::setw(2) << tv.minute << ":" << std::setw(2) << tv.second;
    return oss.str();
}

// Simulated time source with occasional bus time sync delays (e.g., CAN signal)
class TimeSource {
public:
    explicit TimeSource(const StressConfig& c) : cfg(c), rng(rd()) {}

    // update logical time; sometimes mark invalid if last sync too old
    TimeValue next() {
        auto now = Clock::now();

        // advance logical seconds every producer tick
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastTick_);
        if (elapsed.count() >= 1) {
            lastTick_ = now;
            second_ = (second_ + 1) % 60;
            if (second_ == 0) {
                minute_ = (minute_ + 1) % 60;
                if (minute_ == 0) hour_ = (hour_ + 1) % 24;
            }
        }

        // emulate bus sync arrival being late sometimes
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng) < cfg.busDropProbability) {
            // skip sync or make it late
            lastSync_ = now + std::chrono::milliseconds(randRange(0, cfg.busLateMax.count()));
        } else {
            lastSync_ = now; // on-time sync
        }

        TimeValue tv;
        tv.hour = hour_; tv.minute = minute_; tv.second = second_;
        tv.lastUpdate = now;
        // validity: invalid if last sync older than timeout
        tv.valid = ((now - lastSync_) < cfg.timeout);
        return tv;
    }

private:
    StressConfig cfg;
    Clock::time_point lastTick_ = Clock::now();
    Clock::time_point lastSync_ = Clock::now();
    int hour_{12}, minute_{0}, second_{0};

    std::random_device rd;
    std::mt19937 rng;

    int randRange(int a, int b) { // inclusive [a, b]
        std::uniform_int_distribution<int> d(a, b);
        return d(rng);
    }
};

// Naive pipeline: single buffer and immediate blanking on timeout
void runNaive(SimContext& ctx) {
    std::mutex bufMtx;
    TimeValue shared{}; // single buffer shared between producer and consumer
    TimeSource src(ctx.cfg);

    std::thread producer([&] {
        auto period = ctx.cfg.prodPeriod;
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        while (!ctx.stop.load()) {
            auto start = Clock::now();
            // simulate ISR burst delaying producer
            if (dist(rng) < ctx.cfg.isrBusyProbability) {
                auto extra = std::chrono::milliseconds(rand() % (ctx.cfg.isrBusyMax.count()+1));
                std::this_thread::sleep_for(extra);
            }

            TimeValue tv = src.next();
            std::lock_guard<std::mutex> lk(bufMtx); // no exclusive area boundaries beyond plain mutex
            shared = tv;
            ctx.statsNaive.produced++;

            // sleep until next period boundary
            auto elapsed = Clock::now() - start;
            if (elapsed < period) std::this_thread::sleep_for(period - elapsed);
        }
    });

    std::thread consumer([&] {
        auto period = ctx.cfg.dispPeriod;
        while (!ctx.stop.load()) {
            auto start = Clock::now();
            TimeValue snapshot;
            {
                std::lock_guard<std::mutex> lk(bufMtx);
                snapshot = shared; // could be partially updated in real systems without protection
            }

            // naive validity check: blank immediately if timed out
            bool blank = !snapshot.valid || ((Clock::now() - snapshot.lastUpdate) > ctx.cfg.timeout);
            if (blank) ctx.statsNaive.blanks++;
            ctx.statsNaive.consumed++;

            // Print occasionally to visualize blinking
            if (ctx.statsNaive.consumed % 40 == 0) {
                std::cout << "[Naive] " << (blank ? "BLANK --:--:--" : fmt(snapshot)) << "\n";
            }

            auto elapsed = Clock::now() - start;
            if (elapsed < period) std::this_thread::sleep_for(period - elapsed);
        }
    });

    std::this_thread::sleep_for(12s);
    ctx.stop.store(true);
    producer.join();
    consumer.join();
}

// Robust pipeline: double-buffer + exclusive area + grace before blanking
void runRobust(SimContext& ctx) {
    ctx.stop.store(false);

    TimeCache cache;               // double-buffered cache
    ExclusiveArea ex;              // exclusive area to protect multi-field coherency (simulated here)
    TimeSource src(ctx.cfg);

    std::thread producer([&] {
        auto period = ctx.cfg.prodPeriod;
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        while (!ctx.stop.load()) {
            auto start = Clock::now();
            if (dist(rng) < ctx.cfg.isrBusyProbability) {
                auto extra = std::chrono::milliseconds(rand() % (ctx.cfg.isrBusyMax.count()+1));
                std::this_thread::sleep_for(extra);
            }

            TimeValue tv = src.next();
            ex.enter();
            cache.write(tv);
            ex.exit();
            ctx.statsRobust.produced++;

            auto elapsed = Clock::now() - start;
            if (elapsed < period) std::this_thread::sleep_for(period - elapsed);
        }
    });

    std::thread consumer([&] {
        auto period = ctx.cfg.dispPeriod;
        TimeValue lastGood{};
        bool haveLastGood = false;
        Clock::time_point invalidSince{};
        while (!ctx.stop.load()) {
            auto start = Clock::now();

            TimeValue snap = cache.readSnapshot();
            bool validNow = snap.valid && ((Clock::now() - snap.lastUpdate) <= ctx.cfg.timeout);

            if (validNow) {
                lastGood = snap;
                haveLastGood = true;
                invalidSince = {};
            } else {
                if (invalidSince.time_since_epoch().count() == 0) {
                    invalidSince = Clock::now();
                    ctx.statsRobust.invalidTransitions++;
                }
            }

            bool withinGrace = invalidSince.time_since_epoch().count() != 0 &&
                               ((Clock::now() - invalidSince) <= ctx.cfg.grace);

            bool blank = !haveLastGood && !validNow && !withinGrace;
            if (blank) ctx.statsRobust.blanks++;
            ctx.statsRobust.consumed++;

            // Occasionally print
            if (ctx.statsRobust.consumed % 40 == 0) {
                const TimeValue& toShow = (haveLastGood ? lastGood : snap);
                std::cout << "[Robust] "
                          << (blank ? "BLANK --:--:--" : fmt(toShow)) << "\n";
            }

            auto elapsed = Clock::now() - start;
            if (elapsed < period) std::this_thread::sleep_for(period - elapsed);
        }
    });

    std::this_thread::sleep_for(12s);
    ctx.stop.store(true);
    producer.join();
    consumer.join();
}

static void printStats(const char* title, const Stats& s) {
    std::cout << "\n=== " << title << " ===\n";
    std::cout << "Produced: " << s.produced.load() << "\n";
    std::cout << "Consumed: " << s.consumed.load() << "\n";
    std::cout << "Blank events: " << s.blanks.load() << "\n";
    std::cout << "Invalid transitions: " << s.invalidTransitions.load() << "\n";
}

int main(int argc, char** argv) {
    SimContext ctx;
    (void)argc; (void)argv; // suppress unused warnings

    // Allow overriding knobs via env (quick tuning)
    if (const char* e = std::getenv("SIM_TIMEOUT_MS")) ctx.cfg.timeout = std::chrono::milliseconds(std::atoi(e));
    if (const char* e = std::getenv("SIM_GRACE_MS"))   ctx.cfg.grace = std::chrono::milliseconds(std::atoi(e));

    std::cout << "Running NAIVE simulation (12s)..." << std::endl;
    runNaive(ctx);
    printStats("Naive", ctx.statsNaive);

    // Reset stop flag and run robust
    ctx.stop.store(false);
    std::cout << "\nRunning ROBUST simulation (12s)..." << std::endl;
    runRobust(ctx);
    printStats("Robust", ctx.statsRobust);

    std::cout << "\nExpectation: Robust should have far fewer (ideally zero) blank events under the same stress." << std::endl;

    return 0;
}
