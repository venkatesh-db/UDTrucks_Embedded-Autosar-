/* AUTOSAR-style pseudocode for robust time update and display consumption.
 * Map this to your SW-Cs and RTE events. This is illustrative, not compilable against real AUTOSAR libs.
 */

#include <Std_Types.h>

/* Shared cache structure placed in a dedicated memory section if needed */
typedef struct {
    uint8 hour;
    uint8 minute;
    uint8 second;
    boolean valid;
    uint32 lastMonotonicTicks; /* From OsCounter, monotonic */
} ClusterTimeType;

/* Double buffering: two instances */
static ClusterTimeType ClusterTimeBuf[2];
static volatile uint8 ClusterTimeActiveIdx = 0; /* atomic flip, could use exclusive area */

/* Exclusive area wrappers (replace with generated SchM or Rte APIs) */
#define CLUSTER_TIME_EXCLUSIVE_ENTER()  SchM_Enter_ClusterTime_EXCLUSIVE()
#define CLUSTER_TIME_EXCLUSIVE_EXIT()   SchM_Exit_ClusterTime_EXCLUSIVE()

/* Configuration constants */
/* NOTE: Define TIMEBASE_MAX_TICKS per your OsCounter (e.g., 0xFFFFFFFFu for 32-bit) */
#define TIMEBASE_MAX_TICKS           ((uint32)0xFFFFFFFFu)
#define CLUSTER_TIME_TIMEOUT_TICKS   ((uint32)300U)   /* e.g. 300 ms in ticks */
#define CLUSTER_TIME_GRACE_TICKS     ((uint32)200U)   /* debounce grace */

/* Safe delta with wrap-around handling for free-running counters */
static inline uint32 DeltaTicks(uint32 now, uint32 then)
{
    return (now >= then) ? (now - then) : (uint32)( (TIMEBASE_MAX_TICKS - then) + 1u + now );
}

/* State */
static uint32 InvalidSinceTicks = 0U; /* 0 => not invalid yet */
static ClusterTimeType LastGoodValue; /* retain last valid */
static boolean HaveLastGood = FALSE;

/* Producer Runnable: Periodic (e.g., every 100 ms) */
void Rte_Runnable_TimeProducer(void)
{
    ClusterTimeType local;

    /* Acquire current time from source (RTC or COM signal) */
    GetLogicalTime(&local.hour, &local.minute, &local.second);

    /* Validity: evaluate age of last COM sync */
    uint32 nowTicks = Os_GetCounterValue(TimeBaseCounter);
    uint32 syncAge = DeltaTicks(nowTicks, GetLastSyncTicks());
    local.valid = (syncAge < CLUSTER_TIME_TIMEOUT_TICKS);
    local.lastMonotonicTicks = nowTicks;

    /* Write into inactive buffer then flip */
    uint8 next = (uint8)(1U - ClusterTimeActiveIdx);

    CLUSTER_TIME_EXCLUSIVE_ENTER();
    ClusterTimeBuf[next] = local; /* struct copy */
    ClusterTimeActiveIdx = next;  /* publish */
    CLUSTER_TIME_EXCLUSIVE_EXIT();
}

/* Consumer Runnable: Faster Period (e.g., every 50 ms) */
void Rte_Runnable_TimeDisplay(void)
{
    uint32 nowTicks = Os_GetCounterValue(TimeBaseCounter);
    ClusterTimeType snapshot;

    /* Snapshot active buffer */
    uint8 idx = ClusterTimeActiveIdx; /* atomic read */
    snapshot = ClusterTimeBuf[idx];

    boolean validNow = snapshot.valid && (DeltaTicks(nowTicks, snapshot.lastMonotonicTicks) <= CLUSTER_TIME_TIMEOUT_TICKS);

    if (validNow) {
        LastGoodValue = snapshot;
        HaveLastGood = TRUE;
        InvalidSinceTicks = 0U;
    } else {
        if ((InvalidSinceTicks == 0U) && (HaveLastGood == TRUE)) {
            InvalidSinceTicks = nowTicks; /* start grace timing */
        }
    }

    boolean withinGrace = (InvalidSinceTicks != 0U) && (DeltaTicks(nowTicks, InvalidSinceTicks) <= CLUSTER_TIME_GRACE_TICKS);

    boolean blank = (HaveLastGood == FALSE) && (validNow == FALSE) && (withinGrace == FALSE);

    if (!blank) {
        /* Render last good or current snapshot */
        const ClusterTimeType* toDraw = validNow ? &snapshot : &LastGoodValue;
        ClusterDisplay_DrawTime(toDraw->hour, toDraw->minute, toDraw->second);
    } else {
        ClusterDisplay_DrawBlank();
    }
}

/* Quick checklist for integration:
 * - Ensure only TimeProducer writes ClusterTimeBuf.
 * - Use proper SchM exclusive area generated for this module.
 * - On multicore, ensure exclusive area / lock is system-wide (use Spinlock or OS-provided primitive).
 * - Align producer period with desired resolution; guarantee it < timeout.
 * - Validate OsCounter wrap-around; use subtraction that handles modulo if needed.
 * - Tune GRACE vs TIMEOUT to avoid perceptible blink (< human perception ~100-200 ms).
 */
