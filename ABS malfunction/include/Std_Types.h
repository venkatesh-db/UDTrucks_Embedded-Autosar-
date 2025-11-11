/**
 * @file Std_Types.h
 * @brief AUTOSAR standard data types
 * @author Generated for ABS Malfunction Detection System
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

/* Standard AUTOSAR data types */
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned long       uint32;
typedef signed char         sint8;
typedef signed short        sint16;
typedef signed long         sint32;
typedef float               float32;
typedef double              float64;

typedef uint8               boolean;

#ifndef TRUE
#define TRUE                1U
#endif

#ifndef FALSE
#define FALSE               0U
#endif

#ifndef NULL_PTR
#define NULL_PTR            ((void*)0)
#endif

/* AUTOSAR return types */
typedef uint8 Std_ReturnType;

#define E_OK                0x00U
#define E_NOT_OK            0x01U

/* NVM return types */
#define NVM_REQ_OK          0x00U
#define NVM_REQ_NOT_OK      0x01U
#define NVM_REQ_PENDING     0x02U

#endif /* STD_TYPES_H */