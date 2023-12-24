#ifndef __DEFINES_H__
#define __DEFINES_H__

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define __force_inline __attribute__((always_inline))

#ifndef NULL
    #define NULL ((void*) 0)
#endif
#ifndef nullptrGetDrvFb
    #define nullptr (void*) 0
#endif

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;
		
typedef float  float32_t;
typedef double float64_t;

#define RW
#define WO
#define RO                                   const  //read only

#define REG8S(ADDR)                          (*(__IO int8_t*) (ADDR))
#define REG8U(ADDR)                          (*(__IO uint8_t*) (ADDR))
#define REG16S(ADDR)                         (*(__IO int16_t*) (ADDR))
#define REG16U(ADDR)                         (*(__IO uint16_t*) (ADDR))
#define REG32S(ADDR)                         (*(__IO int32_t*) (ADDR))
#define REG32U(ADDR)                         (*(__IO uint32_t*) (ADDR))
#define REG64S(ADDR)                         (*(__IO int64_t*) (ADDR))
#define REG64U(ADDR)                         (*(__IO uint64_t*) (ADDR))

#define ARRAY_SIZE(arr)                      sizeof(arr) / sizeof(*arr)

#define BV(BIT)                              (1 << (BIT))

#define CLRALL(DAT)                          ((DAT) = 0)                  /* clear all bit */
#define SETALL8(DAT)                         ((DAT) = U8_MAX)             /* set all bit(8bit) */
#define SETALL16(DAT)                        ((DAT) = U16_MAX)            /* set all bit(16bit) */
#define SETALL32(DAT)                        ((DAT) = U32_MAX)            /* set all bit(32bit) */
#define SETALL64(DAT)                        ((DAT) = U64_MAX)            /* set all bit(64bit) */
#define SETBIT(DAT, BIT)                     ((DAT) |= (1UL << (BIT)))    /* set a bit */
#define CLRBIT(DAT, BIT)                     ((DAT) &= (~(1UL << (BIT)))) /* clear a bit */
#define CHKBIT(DAT, BIT)                     (((DAT) >> (BIT)) & 0x1)     /* check(get) a bit, align right */

#define SETMSK(DAT, MSK)                     ((DAT) |= (MSK))
#define CLRMSK(DAT, MSK)                     ((DAT) &= ~(MSK))

#define WBIT(DAT, BIT, VAL)                  ((DAT) = ((DAT) & (~(1UL << (BIT)))) | ((VAL) << (BIT)))

#define MASK32(LEN)                          (~(U32_MAX << (LEN)))                     /* mask some bits, STB-start bit, LEN-length */
#define GETBIT32(DAT, STB, LEN)              (((u32) (DAT) >> (STB)) & MASK32(LEN))    /* get some bits, align right */
#define MASK64(LEN)                          (~(U64_MAX << (LEN)))                     /* mask some bits, STB-start bit, LEN-length */
#define GETBIT64(DAT, STB, LEN)              (((u64) (DAT) >> (STB)) & MASK64(LEN))    /* get some bits, align right */
#define LINK32(U16HI, U16LO)                 (((u32) (U16HI) << 16) | ((u16) (U16LO))) /* link two 16bits data to a 32bits data */

#define LINK64_0(U32HI, U32LO)               (((u64) (U32HI) << 32) | ((u32) (U32LO)))                                                 /* link two 32bits data to a 64bits data */
#define LINK64_1(U16W3, U16W2, U16W1, U16W0) (((u64) (U16W3) << 48) | ((u64) (U16W2) << 32) | ((u64) (U16W1) << 16) | ((u64) (U16W0))) /* link four 16bits data to a 64bits data */
#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define LINK64(...)                          GET_MACRO(__VA_ARGS__, LINK64_1, LINK64_0, LINK64_0)(__VA_ARGS__)

#define INCLOSE(val, lhs, rhs)               (((lhs) <= (val)) && ((val) <= (rhs)))
#define INOPEN(val, lhs, rhs)                (((lhs) < (val)) && ((val) < (rhs)))

/*16 bits data decomposition<DATA must be shorter than 16 bits>*/
#define W(DATA)                              (u16)(DATA) /*convert to unsigned short type*/

/*32 bits data decomposition<DATA must be shorter than 32 bits>*/
#define WL(DATA)                             (u16) GETBIT32((u32) DATA, 0, 16)  /*16bits from bit0*/
#define WH(DATA)                             (u16) GETBIT32((u32) DATA, 16, 16) /*16bits from bit16*/

/*64 bits data decomposition<DATA must be shorter than 64 bits>*/
#define W0(DATA)                             (u16) GETBIT64((u64) DATA, 0, 16)  /*16bits from bit0*/
#define W1(DATA)                             (u16) GETBIT64((u64) DATA, 16, 16) /*16bits from bit16*/
#define W2(DATA)                             (u16) GETBIT64((u64) DATA, 32, 16) /*16bits from bit32*/
#define W3(DATA)                             (u16) GETBIT64((u64) DATA, 48, 16) /*16bits from bit48*/

#define M_DEG2RAD                            (3.1415926 / 180)

#endif
