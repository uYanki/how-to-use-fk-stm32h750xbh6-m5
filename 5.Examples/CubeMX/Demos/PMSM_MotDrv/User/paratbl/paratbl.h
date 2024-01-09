#ifndef __PARATBL_H__
#define __PARATBL_H__

#include "main.h"

extern uint8_t u8ParaBuf[];
extern uint8_t u8DbgBuf[];

// 直轴电流Ids Ids：用于产生磁场，与转子的磁场叠加
// 交轴电流Iqs Iqs：：用于控制转矩（其作用等同于直流电机的电枢电流）。
//  iqs=Is ; ids=0时，转矩达到最大值

#define ParaTbl (*(ParaTable_t*)u8ParaBuf)
#define DbgTbl  (*(DebugTable_t*)u8DbgBuf)

typedef struct __packed {
    u64 u64Buf[16];  ///< P0000
    u32 u32Buf[16];  ///< P0064
    u16 u16Buf[16];  ///< P0096
    f32 f32Buf[16];  ///< P0112
} DebugTable_t;

typedef struct __packed {
    u32 u32DrvScheme;  ///< P0000 驱动器方案

    u32 u32SwBuildDate;  ///< P0002 软件构建日期
    u32 u32SwBuildTime;  ///< P0004 软件构建时间
    u32 u32McuSwVer;     ///< P0006

    u16 u16RunState;  ///< P0008

    u16 u16CtrlMode;  ///< P0009
    u16 u16FocMode;   ///< P0010

    /**
     * @brief motor infos
     */
    u16 u16MotType;       ///< P0011 电机类型
    u16 u16MotPolePairs;  ///< P0012

    u16 u16AccMax;     ///< P0013 最大加速度
    u32 u32InertiaWL;  ///< P0014 转动惯量
    u16 u16StatorRes;  ///< P0016 定子电阻
    u16 u16StatorLd;   ///< P0017 定子D轴电感
    u16 u16StatorLq;   ///< P0018 定子Q轴电感

    u16 u16CurRate;   ///< P0019 额定电流
    u16 u16CurMax;    ///< P0020 最大电流
    u16 u16TrqRate;   ///< P0021 额定转矩
    u16 u16TrqMax;    ///< P0022 最大转矩
    u16 u16SpdRate;   ///< P0023 额定转速
    u16 u16SpdMax;    ///< P0024 最大转速
    u16 u16EmfCoeff;  ///< P0025 反电动势常数
    u16 u16TrqCoeff;  ///< P0026 转矩系数
    u16 u16Tm;        ///< P0027 机械时间常数

    /**
     * @brief encoder infos
     */
    u16 u16HallState;     ///< P0028 霍尔真值
    s16 s16MechOffset;    ///< P0029 机械位置偏置
    u16 u16ElecOffset;    ///< P0030 电角度偏置
    u32 u32ElecGearNum;   ///< P0031 电子齿轮比分子
    u32 u32ElecGearDeno;  ///< P0033 电子齿轮比分母

    u16 u16ElecAngSrc;  ///< P0035 电角度来源

    u16 u16EncType;     ///< P0036 编码器类型
    u32 u32EncRes;      ///< P0037 编码器分辨率
    s32 u32EncOffset;   ///< P0039 编码器偏置
    s64 s64EncPosInit;  ///< P0041 编码器上电位置
    s32 s32EncPos;      ///< P0045 编码器单圈位置
    s32 u32EncTurns;    ///< P0047 编码器圈数

    u16 u16EncTurnClr;  ///< P0049 清除编码器多圈值命令

    u16 u16AppSel;  ///< P0050 应用选择

    u16 u16HallAngFb;   ///< P0051 霍尔反馈角度
    u16 u16ElecAngRef;  ///< P0052 电角度指令

    /**
     * @brief foc infos
     */

    s16 s16CurPhAFb;  ///< P0053 A相反馈电流 mA
    s16 s16CurPhBFb;  ///< P0054 B相反馈电流 mA
    s16 s16CurPhCFb;  ///< P0055 C相反馈电流 mA
    s16 s16IalphaFb;  ///< P0056 Alpha轴反馈电流
    s16 s16IbetaFb;   ///< P0057 Beta轴反馈电流
    s16 s16IdFb;      ///< P0059 D轴反馈电流
    s16 s16IqFb;      ///< P0058 Q轴反馈电流

    s16 s16VdRef;      ///< P0060 D轴电压指令
    s16 s16VqRef;      ///< P0061 Q轴电压指令
    s16 s16ValphaRef;  ///< P0062 Alpha轴电压指令
    s16 s16VbetaRef;   ///< P0063 Beta轴电压指令
    s16 s16VphaRef;    ///< P0064 A相电压指令
    s16 s16VphbRef;    ///< P0065 B相电压指令
    s16 s16VphcRef;    ///< P0066 C相电压指令
    u16 u16DutyPha;    ///< P0067 A相占空比指令
    u16 u16DutyPhb;    ///< P0068 B相占空比指令
    u16 u16DutyPhc;    ///< P0069 C相占空比指令
    u16 u16Sector;     ///< P0070 输出电压矢量扇区

    u16 u16HallAngFb2;  ///< P0071 电角度指令
    u16 u16Umdc;        ///< P0072 母线电压 0.1v
    u16 u16ElecAngFb;   ///< P0073 电角度反馈
    u16 u16MechAngFb;   ///< P0074 机械角度反馈
    u16 u16CarryFreq;   ///< P0075 载波频率
    u16 u16PwmDutyMax;  ///< P0076 占空比最大值
    u16 u16MechAngRef;  ///< P0077 机械角度指令 0~360

    s16 s16AiU;     ///< P0078 模拟量输入, unit: mv
    s16 s16AiBias;  ///< P0079 模拟量偏置, unit: mv

    s16 s16SpdDigRef;       ///< P0080 数字速度指令
    u16 u16SpdMulRefSel;    ///< P0081 多段数字速度指令选择
    s16 s16SpdDigRefs[16];  ///< P0082 多段数字速度指令

    u16 u16PosTgtSrc;       ///< P0098 位置指令来源
    s64 s64PosDigRef;       ///< P0099 数字位置指令
    u16 u16PosMulRefSel;    ///< P0103 多段数字位置指令选择
    s64 s64PosDigRefs[16];  ///< P0104 多段数字位置速度指令

    u16 u16PosLimSrc;  ///< P0168 位置限制来源
    u16 u16PosLimRes;  ///< P0169 位置限制状态
    u16 s64PosFwdLim;  ///< P0170 正向位置限制
    u16 s64PosRevLim;  ///< P0171 反向位置限制

    s16 s16TrqDigRef;       ///< P0172 数字转矩指令
    u16 u16TrqMulRefSel;    ///< P0173 多段数字转矩指令选择
    s16 s16TrqDigRefs[16];  ///< P0174 多段数字转矩指令

    s16 s16SpdTgt;  ///< P0190 当前速度指令
    s64 s64PosTgt;  ///< P0191 当前位置指令
    s16 s16TrqTgt;  ///< P0195 当前转矩指令

    u16 u16ElecAngInc;   ///< P0196 开环电角度自增量
    u16 u16WaitTimeInc;  ///< P0197 开环电角度自增间隔 (us)
    s16 s16SpdFb;        ///< P0198 速度反馈 RPM
    s64 s64PosFb;        ///< P0199 位置反馈 Pulse
    s16 s16TrqFb;        ///< P0203 转矩反馈

    u16 u16EncFreqDivDir;   ///< P0204 编码器分频输出脉冲方向
    u16 u16EncFreqDivNum;   ///< P0205 编码器分频输出分子
    u16 u16EncFreqDivDeno;  ///< P0206 编码器分频输出分母

    /**
     * @brief Curve Trace
     */

    u16 u16LogSampPrd;   ///< P0207 数据记录采样周期设定
    u16 u16LogSampPts;   ///< P0208 数据记录采样点数设定
    u16 u16LogChCnt;     ///< P0209 数据记录通道数量设定
    u16 u16LogChSrc[8];  ///< P0210 数据记录通道地址设定 (数据源)

    int16_t s16IaBias;
    int16_t s16IbBias;
    int16_t s16IcBias;

} ParaTable_t;

typedef struct __packed {
    s32 s32LogChBuf[2000 * 8];
} CurveTraceBuffer_t;

void ParaTbl_Init(void);

#endif
