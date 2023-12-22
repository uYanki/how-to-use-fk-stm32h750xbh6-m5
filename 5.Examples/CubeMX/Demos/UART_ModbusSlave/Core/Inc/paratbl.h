#ifndef __PARATBL_H__
#define __PARATBL_H__

#include "defs.h"

extern uint8_t u8ParaBuf[];
extern uint8_t u8DbgBuf[];

#define ParaTbl (*(ParaTable_t*) u8ParaBuf)
#define DbgTbl  (*(DebugTable_t*) u8DbgBuf)

//

typedef enum {
    OPENLOOP_CTRL_MODE_SPD,
    CLOSELOOP_CTRL_MODE_TRQ,
    CLOSELOOP_CTRL_MODE_SPD,
    CLOSELOOP_CTRL_MODE_POS,
} ctrl_mode_t;

typedef __packed struct {
    u64 u64Buf[16];
    u32 u32Buf[16];
    u16 u16Buf[16];
    f32 f32Buf[16];
} DebugTable_t;

typedef __packed struct {

    u32 u32DrvScheme;    ///< 驱动器方案
    u16 u16SwVerMajor;   ///< 软件基线版本号
    u16 u16SwVerMinor;   ///< 软件分支版本号
    u32 u32SwBuildDate;  ///< 软件构建日期
    u32 u32SwBuildTime;  ///< 软件构建时间

    u16 u16RunState;  ///<

    u16 u16CtrlMode;  ///<

    /**
     * @brief motor infos
     */
    u16 u16MotType;       ///< 电机类型
    u16 u16MotPolePairs;  ///<

    // u16 u16Ls;
    // u16 u16Rs;

    u16 u16CurRate;  ///< 额定电流
    u16 u16CurMax;   ///< 最大电流
    u16 u16TrqRate;  ///< 额定转矩
    u16 u16TrqMax;   ///< 最大转矩
    u16 u16SpdRate;  ///< 额定转速
    u16 u16SpdMax;   ///< 最大转速

    /**
     * @brief encoder infos
     */
    u16 u16HallState;   ///<
    u16 u16HallOffset;  ///<

    u16 u16EncType;    ///<
    u32 u32EncRes;     ///<
    s32 u32EncPos;     ///<
    s32 u32EncTurns;   ///<
    s32 u32EncOffset;  ///<

    /**
     * @brief encoder infos
     */

    s16 s16SpdRef;  ///<
    s16 s16SpdFb;   ///<
    s64 s64PosRef;  ///<
    s64 s64PosFb;   ///<
    s16 s16TrqRef;  ///<
    s16 s16TrqFb;   ///<

    /**
     * @brief foc infos
     */

    s16 s16CurPhAFb;  ///< A相反馈电流 mA
    s16 s16CurPhBFb;  ///< B相反馈电流 mA
    s16 s16CurPhCFb;  ///< C相反馈电流 mA
    s16 s16IalphaFb;  ///< Alpha轴反馈电流
    s16 s16IbetaFb;   ///< Beta轴反馈电流
    s16 s16IqFb;      ///< Q轴反馈电流
    s16 s16IdFb;      ///< D轴反馈电流

    s16 s16VqRef;        ///< Q轴电压指令
    s16 s16VdRef;        ///< D轴电压指令
    s16 s16ValphaRef;    ///< Alpha轴电压指令
    s16 s16VbetaRef;     ///< Beta轴电压指令
    u16 u16SvpwmTa;      ///< A相占空比指令
    u16 u16SvpwmTb;      ///< B相占空比指令
    u16 u16SvpwmTc;      ///< C相占空比指令
    u16 u16SvpwmSector;  ///< 输出电压矢量扇区
    u16 u16ElecAngRef;   ///< 电角度指令
    u16 u16Umdc_C;       ///< 控制电电压 0.1v, L1C & L2C
    u16 u16Umdc_D;       ///< 动力电电压 0.1v, L1 & L2
    u16 u16ElecAngFb;    ///< 电角度反馈
    u16 u16MechAngFb;    ///< 机械角度反馈
    u16 u16TickCount;    ///< 1s内速度环中断计数
    u16 u16CarryFreq;    ///< 载波频率
    u16 u16PwmDutyMax;   ///< 占空比最大值

} ParaTable_t;

#endif
