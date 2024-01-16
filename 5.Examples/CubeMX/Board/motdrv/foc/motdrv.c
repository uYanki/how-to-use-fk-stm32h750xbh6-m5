#include "motdrv.h"
#include "pid.h"

/**
 * @brief LA034-040NN07A
 * 极对数：手转一圈电机，两相间出现的正弦波数
 * 相间电阻(三粗线): 13.5~13.8
 * 线电阻 = 相间电阻/2 = 6.75
 *
 */

// https://blog.csdn.net/weixin_42887190/article/details/124289307

// 移相: https://shequ.stmicroelectronics.cn/thread-612527-1-1.html

/**
 *  Ia = Im * cos(θ)
 *  Ib = Im * cos(θ-2π/3)
 *  Ic = Im * cos(θ-4π/3)
 *     = Im * cos(θ+2π/3)
 *
 *  Ia + Ib + Ic = 0 (基尔霍夫定律)
 *
 */

#include "usbd_cdc_if.h"
#include "tim.h"

#define Q15(n) (int16_t)(32768 * (n))

// sincostab16 in 10-bit resolution
static const int16_t sincostab[1024] = {
    0, 201, 402, 603, 804, 1005, 1206, 1407, 1608, 1809,
    2009, 2210, 2410, 2611, 2811, 3012, 3212, 3412, 3612, 3811,
    4011, 4210, 4410, 4609, 4808, 5007, 5205, 5404, 5602, 5800,
    5998, 6195, 6393, 6590, 6786, 6983, 7179, 7375, 7571, 7767,
    7962, 8157, 8351, 8545, 8739, 8933, 9126, 9319, 9512, 9704,
    9896, 10087, 10278, 10469, 10659, 10849, 11039, 11228, 11417, 11605,
    11793, 11980, 12167, 12353, 12539, 12725, 12910, 13094, 13279, 13462,
    13645, 13828, 14010, 14191, 14372, 14553, 14732, 14912, 15090, 15269,
    15446, 15623, 15800, 15976, 16151, 16325, 16499, 16673, 16846, 17018,
    17189, 17360, 17530, 17700, 17869, 18037, 18204, 18371, 18537, 18703,
    18868, 19032, 19195, 19357, 19519, 19680, 19841, 20000, 20159, 20317,
    20475, 20631, 20787, 20942, 21096, 21250, 21403, 21554, 21705, 21856,
    22005, 22154, 22301, 22448, 22594, 22739, 22884, 23027, 23170, 23311,
    23452, 23592, 23731, 23870, 24007, 24143, 24279, 24413, 24547, 24680,
    24811, 24942, 25072, 25201, 25329, 25456, 25582, 25708, 25832, 25955,
    26077, 26198, 26319, 26438, 26556, 26674, 26790, 26905, 27019, 27133,
    27245, 27356, 27466, 27575, 27683, 27790, 27896, 28001, 28105, 28208,
    28310, 28411, 28510, 28609, 28706, 28803, 28898, 28992, 29085, 29177,
    29268, 29358, 29447, 29534, 29621, 29706, 29791, 29874, 29956, 30037,
    30117, 30195, 30273, 30349, 30424, 30498, 30571, 30643, 30714, 30783,
    30852, 30919, 30985, 31050, 31113, 31176, 31237, 31297, 31356, 31414,
    31470, 31526, 31580, 31633, 31685, 31736, 31785, 31833, 31880, 31926,
    31971, 32014, 32057, 32098, 32137, 32176, 32213, 32250, 32285, 32318,
    32351, 32382, 32412, 32441, 32469, 32495, 32521, 32545, 32567, 32589,
    32609, 32628, 32646, 32663, 32678, 32692, 32705, 32717, 32728, 32737,
    32745, 32752, 32757, 32761, 32765, 32766, 32767, 32766, 32765, 32761,
    32757, 32752, 32745, 32737, 32728, 32717, 32705, 32692, 32678, 32663,
    32646, 32628, 32609, 32589, 32567, 32545, 32521, 32495, 32469, 32441,
    32412, 32382, 32351, 32318, 32285, 32250, 32213, 32176, 32137, 32098,
    32057, 32014, 31971, 31926, 31880, 31833, 31785, 31736, 31685, 31633,
    31580, 31526, 31470, 31414, 31356, 31297, 31237, 31176, 31113, 31050,
    30985, 30919, 30852, 30783, 30714, 30643, 30571, 30498, 30424, 30349,
    30273, 30195, 30117, 30037, 29956, 29874, 29791, 29706, 29621, 29534,
    29447, 29358, 29268, 29177, 29085, 28992, 28898, 28803, 28706, 28609,
    28510, 28411, 28310, 28208, 28105, 28001, 27896, 27790, 27683, 27575,
    27466, 27356, 27245, 27133, 27019, 26905, 26790, 26674, 26556, 26438,
    26319, 26198, 26077, 25955, 25832, 25708, 25582, 25456, 25329, 25201,
    25072, 24942, 24811, 24680, 24547, 24413, 24279, 24143, 24007, 23870,
    23731, 23592, 23452, 23311, 23170, 23027, 22884, 22739, 22594, 22448,
    22301, 22154, 22005, 21856, 21705, 21554, 21403, 21250, 21096, 20942,
    20787, 20631, 20475, 20317, 20159, 20000, 19841, 19680, 19519, 19357,
    19195, 19032, 18868, 18703, 18537, 18371, 18204, 18037, 17869, 17700,
    17530, 17360, 17189, 17018, 16846, 16673, 16499, 16325, 16151, 15976,
    15800, 15623, 15446, 15269, 15090, 14912, 14732, 14553, 14372, 14191,
    14010, 13828, 13645, 13462, 13279, 13094, 12910, 12725, 12539, 12353,
    12167, 11980, 11793, 11605, 11417, 11228, 11039, 10849, 10659, 10469,
    10278, 10087, 9896, 9704, 9512, 9319, 9126, 8933, 8739, 8545,
    8351, 8157, 7962, 7767, 7571, 7375, 7179, 6983, 6786, 6590,
    6393, 6195, 5998, 5800, 5602, 5404, 5205, 5007, 4808, 4609,
    4410, 4210, 4011, 3811, 3612, 3412, 3212, 3012, 2811, 2611,
    2410, 2210, 2009, 1809, 1608, 1407, 1206, 1005, 804, 603,
    402, 201, 0, -201, -402, -603, -804, -1005, -1206, -1407,
    -1608, -1809, -2009, -2210, -2410, -2611, -2811, -3012, -3212, -3412,
    -3612, -3811, -4011, -4210, -4410, -4609, -4808, -5007, -5205, -5404,
    -5602, -5800, -5998, -6195, -6393, -6590, -6786, -6983, -7179, -7375,
    -7571, -7767, -7962, -8157, -8351, -8545, -8739, -8933, -9126, -9319,
    -9512, -9704, -9896, -10087, -10278, -10469, -10659, -10849, -11039, -11228,
    -11417, -11605, -11793, -11980, -12167, -12353, -12539, -12725, -12910, -13094,
    -13279, -13462, -13645, -13828, -14010, -14191, -14372, -14553, -14732, -14912,
    -15090, -15269, -15446, -15623, -15800, -15976, -16151, -16325, -16499, -16673,
    -16846, -17018, -17189, -17360, -17530, -17700, -17869, -18037, -18204, -18371,
    -18537, -18703, -18868, -19032, -19195, -19357, -19519, -19680, -19841, -20000,
    -20159, -20317, -20475, -20631, -20787, -20942, -21096, -21250, -21403, -21554,
    -21705, -21856, -22005, -22154, -22301, -22448, -22594, -22739, -22884, -23027,
    -23170, -23311, -23452, -23592, -23731, -23870, -24007, -24143, -24279, -24413,
    -24547, -24680, -24811, -24942, -25072, -25201, -25329, -25456, -25582, -25708,
    -25832, -25955, -26077, -26198, -26319, -26438, -26556, -26674, -26790, -26905,
    -27019, -27133, -27245, -27356, -27466, -27575, -27683, -27790, -27896, -28001,
    -28105, -28208, -28310, -28411, -28510, -28609, -28706, -28803, -28898, -28992,
    -29085, -29177, -29268, -29358, -29447, -29534, -29621, -29706, -29791, -29874,
    -29956, -30037, -30117, -30195, -30273, -30349, -30424, -30498, -30571, -30643,
    -30714, -30783, -30852, -30919, -30985, -31050, -31113, -31176, -31237, -31297,
    -31356, -31414, -31470, -31526, -31580, -31633, -31685, -31736, -31785, -31833,
    -31880, -31926, -31971, -32014, -32057, -32098, -32137, -32176, -32213, -32250,
    -32285, -32318, -32351, -32382, -32412, -32441, -32469, -32495, -32521, -32545,
    -32567, -32589, -32609, -32628, -32646, -32663, -32678, -32692, -32705, -32717,
    -32728, -32737, -32745, -32752, -32757, -32761, -32765, -32766, -32767, -32766,
    -32765, -32761, -32757, -32752, -32745, -32737, -32728, -32717, -32705, -32692,
    -32678, -32663, -32646, -32628, -32609, -32589, -32567, -32545, -32521, -32495,
    -32469, -32441, -32412, -32382, -32351, -32318, -32285, -32250, -32213, -32176,
    -32137, -32098, -32057, -32014, -31971, -31926, -31880, -31833, -31785, -31736,
    -31685, -31633, -31580, -31526, -31470, -31414, -31356, -31297, -31237, -31176,
    -31113, -31050, -30985, -30919, -30852, -30783, -30714, -30643, -30571, -30498,
    -30424, -30349, -30273, -30195, -30117, -30037, -29956, -29874, -29791, -29706,
    -29621, -29534, -29447, -29358, -29268, -29177, -29085, -28992, -28898, -28803,
    -28706, -28609, -28510, -28411, -28310, -28208, -28105, -28001, -27896, -27790,
    -27683, -27575, -27466, -27356, -27245, -27133, -27019, -26905, -26790, -26674,
    -26556, -26438, -26319, -26198, -26077, -25955, -25832, -25708, -25582, -25456,
    -25329, -25201, -25072, -24942, -24811, -24680, -24547, -24413, -24279, -24143,
    -24007, -23870, -23731, -23592, -23452, -23311, -23170, -23027, -22884, -22739,
    -22594, -22448, -22301, -22154, -22005, -21856, -21705, -21554, -21403, -21250,
    -21096, -20942, -20787, -20631, -20475, -20317, -20159, -20000, -19841, -19680,
    -19519, -19357, -19195, -19032, -18868, -18703, -18537, -18371, -18204, -18037,
    -17869, -17700, -17530, -17360, -17189, -17018, -16846, -16673, -16499, -16325,
    -16151, -15976, -15800, -15623, -15446, -15269, -15090, -14912, -14732, -14553,
    -14372, -14191, -14010, -13828, -13645, -13462, -13279, -13094, -12910, -12725,
    -12539, -12353, -12167, -11980, -11793, -11605, -11417, -11228, -11039, -10849,
    -10659, -10469, -10278, -10087, -9896, -9704, -9512, -9319, -9126, -8933,
    -8739, -8545, -8351, -8157, -7962, -7767, -7571, -7375, -7179, -6983,
    -6786, -6590, -6393, -6195, -5998, -5800, -5602, -5404, -5205, -5007,
    -4808, -4609, -4410, -4210, -4011, -3811, -3612, -3412, -3212, -3012,
    -2811, -2611, -2410, -2210, -2009, -1809, -1608, -1407, -1206, -1005,
    -804, -603, -402, -201,
};

void clarke(clarke_t* v)
{
    v->alpha = v->phase_a;
    v->beta  = (v->phase_a + v->phase_b * 2) * M_INVSQRT3;
}

void iclarke(iclarke_t* v)
{
    v->phase_a = v->alpha;
    v->phase_b = +0.5 * (M_SQRT3 * v->beta - v->alpha);
    v->phase_c = -0.5 * (M_SQRT3 * v->beta + v->alpha);
}

void park(park_t* v)
{
    v->d = v->alpha * v->cos + v->beta * v->sin;
    v->q = v->beta * v->cos - v->alpha * v->sin;
}

void ipark(ipark_t* v)
{
    v->alpha = v->d * v->cos - v->q * v->sin;
    v->beta  = v->d * v->sin + v->q * v->cos;
}

void minmax(svpwm_t* v)
{
    if (v->phase_a >= v->phase_b)
    {
        if (v->phase_a >= v->phase_c)
        {
            if (v->phase_b >= v->phase_c)
            {
                v->sector    = 1;
                v->phase_max = v->phase_a;
                v->phase_mid = v->phase_b;
                v->phase_min = v->phase_c;
            }
            else
            {
                v->sector    = 6;
                v->phase_max = v->phase_a;
                v->phase_mid = v->phase_c;
                v->phase_min = v->phase_b;
            }
        }
        else
        {
            v->sector    = 5;
            v->phase_max = v->phase_c;
            v->phase_mid = v->phase_a;
            v->phase_min = v->phase_b;
        }
    }
    else
    {
        if (v->phase_a < v->phase_c)
        {
            if (v->phase_b < v->phase_c)
            {
                v->sector    = 4;
                v->phase_max = v->phase_c;
                v->phase_mid = v->phase_b;
                v->phase_min = v->phase_a;
            }
            else
            {
                v->sector    = 3;
                v->phase_max = v->phase_b;
                v->phase_mid = v->phase_c;
                v->phase_min = v->phase_a;
            }
        }
        else
        {
            v->sector    = 2;
            v->phase_max = v->phase_b;
            v->phase_mid = v->phase_a;
            v->phase_min = v->phase_c;
        }
    }
}

void zero_inject(svpwm_t* v)
{
    // https://blog.csdn.net/happy_baymax/article/details/124328899
    // https://blog.csdn.net/weixin_51545907/article/details/131612824

    // 高频注入的本质，引入一个周期与原信号周期的倍数关系的信号，所有相电压同加/同减同一个值，相间压差不变。

    float32_t V0;

    V0 = -0.5 * (v->phase_min + v->phase_max);  // 均值

    // V0 = -1 - v->phase_min;  // 极小值
    // V0 = 1 - v->phase_max;   // 极大值

    // V0 = (abs(v->phase_max) >= abs(v->phase_min)) ? (1 - v->phase_max) : (-1 - v->phase_min);  // 交替

    v->Ta = v->phase_a + V0;
    v->Tb = v->phase_b + V0;
    v->Tc = v->phase_c + V0;

    // [-32768,+32767] => [-0.5,+0.5]
    v->Ta /= 65535;
    v->Tb /= 65535;
    v->Tc /= 65535;

    // [-0.5,0.5] => [0,1]
    v->Ta += 0.5;
    v->Tb += 0.5;
    v->Tc += 0.5;

    v->Ta *= v->period;
    v->Tb *= v->period;
    v->Tc *= v->period;
}

void sincos(sincos_t* v)
{
    // 10-bit resolution
    uint16_t p = v->angle >> 6;  // 0~1023

    // [-1,1] => [-32678,32767]
    v->sin = sincostab[p];        // 0~1023
    v->cos = sincostab[p & 0x03FF]; 
}

void svpwm7_2(svpwm_t* v, iclarke_t* k)  // 和下方的 svpwm7 是完全等效的
{
    f32 X = k->beta;
    f32 Y = (k->beta + M_SQRT3 * k->alpha) * 0.5f;
    f32 Z = Y - X;

    v->sector = 3;

    if (Y > 0)
    {
        v->sector -= 1;
    }
    if (Z > 0)
    {
        v->sector -= 1;
    }
    if (X < 0)
    {
        v->sector = 7 - v->sector;
    }

    switch (v->sector)
    {
        case 1:
        case 4: {
            v->Ta = Y;
            v->Tb = X - Z;
            v->Tc = -Y;
            break;
        }
        case 2:
        case 5: {
            v->Ta = Z + Y;
            v->Tb = X;
            v->Tc = -X;
            break;
        }
        default:
        case 3:
        case 6: {
            v->Ta = Z;
            v->Tb = -Z;
            v->Tc = -(X + Y);
            break;
        }
    }

    // [-32768,+32767] => [-0.5,+0.5]
    v->Ta /= 65535;
    v->Tb /= 65535;
    v->Tc /= 65535;

    // [-0.5,0.5] => [0,1]
    v->Ta += 0.5;
    v->Tb += 0.5;
    v->Tc += 0.5;

    v->Ta *= v->period;
    v->Tb *= v->period;
    v->Tc *= v->period;
}

#include "system/sleep.h"

void svpwm7(svpwm_t* v, iclarke_t* k)
{
    // Svpwm Sector Judgment

    float32_t u1 = k->beta;
    float32_t u2 = +0.5 * (M_SQRT3 * k->alpha - k->beta);
    float32_t u3 = -0.5 * (M_SQRT3 * k->alpha + k->beta);

    uint8_t a = (u1 > 0) ? 1 : 0;
    uint8_t b = (u2 > 0) ? 1 : 0;
    uint8_t c = (u3 > 0) ? 1 : 0;

    uint8_t n = (c << 2) + (b << 1) + (a << 0);

    switch (n)
    {
        case 3: v->sector = 1; break;
        case 1: v->sector = 2; break;
        case 5: v->sector = 3; break;
        case 4: v->sector = 4; break;
        case 6: v->sector = 5; break;
        case 2: v->sector = 6; break;
    }

    // Get Vector Duration

    u1 /= 32768;
    u2 /= 32768;
    u3 /= 32768;

    float32_t t0, t1, t2, t3, t4, t5, t6;
    float32_t ta, tb, tc;

    switch (v->sector)
    {
        case 1:
            t4 = u2;
            t6 = u1;
            t0 = 1.0f - t4 - t6;
            break;
        case 2:
            t2 = -u2;
            t6 = -u3;
            t0 = 1.0f - t2 - t6;
            break;
        case 3:
            t2 = u1;
            t3 = u3;
            t0 = 1.0f - t2 - t3;
            break;
        case 4:
            t1 = -u1;
            t3 = -u2;
            t0 = 1.0f - t1 - t3;
            break;
        case 5:
            t1 = u3;
            t5 = u2;
            t0 = 1.0f - t1 - t5;
            break;
        case 6:
            t4 = -u3;
            t5 = -u1;
            t0 = 1.0f - t4 - t5;
            break;
    }

    t0 /= 2.0f;

    // Svpwm Generate

    switch (v->sector)
    {
        case 1:
            ta = t4 + t6 + t0;
            tb = t6 + t0;
            tc = t0;
            break;
        case 2:
            ta = t6 + t0;
            tb = t2 + t6 + t0;
            tc = t0;
            break;
        case 3:
            ta = t0;
            tb = t2 + t3 + t0;
            tc = t3 + t0;
            break;
        case 4:
            ta = t0;
            tb = t3 + t0;
            tc = t1 + t3 + t0;
            break;
        case 5:
            ta = t5 + t0;
            tb = t0;
            tc = t1 + t5 + t0;
            break;
        case 6:
            ta = t4 + t5 + t0;
            tb = t0;
            tc = t5 + t0;
            break;
        default:
            ta = tb = tc = 0.5f;
            break;
    }

    v->Ta = ta * v->period;
    v->Tb = tb * v->period;
    v->Tc = tc * v->period;
}

static PID_t SpdPid = {.Kp = 0.005, .Ki = 0.001, .Kd = 0.0, .ref = 0, .ramp = 0, .lo = 0, .hi = 32767, .Ts = 1.f / 8e3};
static PID_t IdPid  = {.Kp = 0.001, .Ki = 0.003, .Kd = 0.0, .ref = 0, .ramp = 10000, .lo = -32768, .hi = 32767, .Ts = 1.f / 2e3};
static PID_t IqPid  = {.Kp = 0.001, .Ki = 0.003, .Kd = 0.0, .ref = 0, .ramp = 10000, .lo = -32768, .hi = 32767, .Ts = 1.f / 2e3};

void spdloop()
{
#if 1
    if (ParaTbl.u16ElecAngSrc != ELEC_ANG_SRC_NONE)
    {
        ParaTbl.s16SpdTgt = ParaTbl.s16SpdDigRef;

        SpdPid.fbk = ParaTbl.s16SpdFb;
        SpdPid.ref = ParaTbl.s16SpdTgt;
        PID_Handler_Tustin(&SpdPid);

        ParaTbl.s16VqRef = SpdPid.out;

        DelayBlockUS(125);  // Speed Loop 8kHz
    }

#else

#endif
}

static int curloop_i = 0;

void curloop(void)
{
#if 0

    if (++curloop_i < 4)  // 4kHz
    {
        return;
    }
    else
    {
        curloop_i = 0;
    }

    // IdPid.fbk = ParaTbl.s16IdFb;
    // IdPid.ref = ParaTbl.s16VdRef;
    // PID_Handler_Tustin(&IdPid);

    // ParaTbl.s16VdRef = IdPid.out;

    IqPid.fbk = ParaTbl.s16IqFb;
    IqPid.ref = ParaTbl.s16VqRef;
    PID_Handler_Tustin(&IqPid);

    ParaTbl.s16VqRef = IqPid.out;

#else

#endif
}

void ifoc(void)
{
    clarke_t clarke_v = {
        .phase_a = ParaTbl.s16CurPhAFb,
        .phase_b = ParaTbl.s16CurPhBFb,
        .phase_c = ParaTbl.s16CurPhCFb,
    };

    clarke(&clarke_v);

    sincos_t sincos_v = {
        .angle = ParaTbl.u16ElecAngRef,
    };

    sincos(&sincos_v);

    park_t park_v = {

        .alpha = clarke_v.alpha,
        .beta  = clarke_v.beta,

        .sin = sincos_v.sin / 32768.f,
        .cos = sincos_v.cos / 32768.f,
    };

    park(&park_v);

    ParaTbl.s16IdFb = park_v.d;
    ParaTbl.s16IqFb = park_v.q;
}

static volatile int i = 0;

void ofoc(void)
{
    sincos_t sincos_v = {
        .angle = ParaTbl.u16ElecAngRef,
    };

    sincos(&sincos_v);

    ipark_t ipark_v = {
        .d   = ParaTbl.s16VdRef,
        .q   = ParaTbl.s16VqRef,
        .sin = sincos_v.sin / 32768.f,
        .cos = sincos_v.cos / 32768.f,
    };

    ipark(&ipark_v);

    iclarke_t iclarke_v = {
        .alpha = ipark_v.alpha,
        .beta  = ipark_v.beta,
    };

    iclarke(&iclarke_v);

    svpwm_t svpwm_v = {
        .phase_a = iclarke_v.phase_a,
        .phase_b = iclarke_v.phase_b,
        .phase_c = iclarke_v.phase_c,
        .period  = ParaTbl.u16PwmDutyMax,
    };

    minmax(&svpwm_v);
    ParaTbl.u16Sector = svpwm_v.sector;

#if 0
   
    zero_inject(&svpwm_v);
#else

    // svpwm7(&svpwm_v, &iclarke_v);
    // usb_printf("%d,%d,%d", (s16)svpwm_v.Ta, (s16)svpwm_v.Tb, (s16)svpwm_v.Tc);
    svpwm7_2(&svpwm_v, &iclarke_v);
    // usb_printf(",%d,%d,%d\n", (s16)svpwm_v.Ta, (s16)svpwm_v.Tb, (s16)svpwm_v.Tc);

    // overmod
#endif

    ParaTbl.u16DutyPha = svpwm_v.Ta;
    ParaTbl.u16DutyPhb = svpwm_v.Tb;
    ParaTbl.u16DutyPhc = svpwm_v.Tc;

    PWM_SetDuty(svpwm_v.Ta, svpwm_v.Tb, svpwm_v.Tc);

    ADC2->CFGR &= ~(0x11 << 10u);
    ADC2->CFGR |= ADC_EXTERNALTRIGCONVEDGE_RISING;

#define SHUNT 1

#if SHUNT == 1
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_4, min(ParaTbl.u16DutyPha, min(ParaTbl.u16DutyPhb, ParaTbl.u16DutyPhc)));
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    i = 1;
#elif SHUNT == 3
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_4, svpwm_v.period - 10);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
#endif
}

#include "adc.h"
volatile int n = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC2)
    {
#if SHUNT == 1
        if (i == 1)
        {
            ADC2->CFGR &= ~(0x11 << 10u);
            ADC2->CFGR |= ADC_EXTERNALTRIGCONVEDGE_FALLING;
            __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_4, max(ParaTbl.u16DutyPha, min(ParaTbl.u16DutyPhb, ParaTbl.u16DutyPhc)));
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
            i = 2;
        }
        else if (i == 2)
        {
            i = 3;
            extern void cur_rs();
            cur_rs();
        }
#elif SHUNT == 3
        extern void cur_rs2();
        cur_rs2();
#endif
    }
}

#include "motdrv/enc/hall.h"

void current_reconstruct(uint16_t Iz[2])  // shunt1
{
    int16_t temp0 = Iz[0], temp1 = Iz[1], Ibias = 0;

    s16 s16CurPhAFb, s16CurPhBFb, s16CurPhCFb;

    // int16_t Ibus1 = Iz[0] - Ibias;
    // int16_t Ibus2 = Iz[1] - Ibias;

    s16CurPhAFb              = temp0;
    s16CurPhCFb              = 0 - temp1;
    s16CurPhBFb              = 0 - s16CurPhAFb - s16CurPhCFb;
    *(s16*)&DbgTbl.u16Buf[0] = s16CurPhAFb;

    s16CurPhBFb              = temp0;
    s16CurPhCFb              = 0 - temp1;
    s16CurPhAFb              = 0 - s16CurPhBFb - s16CurPhCFb;
    *(s16*)&DbgTbl.u16Buf[1] = s16CurPhAFb;

    s16CurPhBFb              = temp0;
    s16CurPhAFb              = 0 - temp1;
    s16CurPhCFb              = 0 - s16CurPhAFb - s16CurPhBFb;
    *(s16*)&DbgTbl.u16Buf[2] = s16CurPhAFb;

    s16CurPhCFb              = temp0;
    s16CurPhAFb              = 0 - temp1;
    s16CurPhBFb              = 0 - s16CurPhCFb - s16CurPhAFb;
    *(s16*)&DbgTbl.u16Buf[3] = s16CurPhAFb;
    s16CurPhCFb              = temp0;
    s16CurPhBFb              = 0 - temp1;
    s16CurPhAFb              = 0 - s16CurPhCFb - s16CurPhBFb;
    *(s16*)&DbgTbl.u16Buf[4] = s16CurPhAFb;

    s16CurPhAFb              = temp0;
    s16CurPhBFb              = 0 - temp1;
    s16CurPhCFb              = 0 - s16CurPhAFb - s16CurPhBFb;
    *(s16*)&DbgTbl.u16Buf[5] = s16CurPhAFb;

    // int16_t Ia = 0, Ib = 0, Ic = 0;

    // // 3  -1+5=7  %6 = 1

    // int a = ParaTbl.u16Sector;

    // extern hall_encoder_t HallEnc;

    // a = HallEnc.HallState;

    // int c = DbgTbl.u16Buf[0];

    //     // if (a == b[c][0])
    //     {
    //         Ia = Ibus1;
    //         Ic = -Ibus2;
    //         Ib = -Ia - Ic;

    //         *(s16*)&DbgTbl.u16Buf[0] = Ia;
    //     }
    //     // if (a == b[c][1])
    //     {
    //         Ic = Ibus1;
    //         Ia = -Ibus2;
    //         Ib = -Ia - Ic;

    //         *(s16*)&DbgTbl.u16Buf[1] = Ia;
    //     }
    //     // if (a == b[c][2])
    //     {
    //         Ib = Ibus1;
    //         Ia = -Ibus2;
    //         Ic = -Ia - Ib;

    //         *(s16*)&DbgTbl.u16Buf[2] = Ia;
    //     }
    //     // if (a == b[c][3])
    //     {
    //         Ia = Ibus1;
    //         Ib = -Ibus2;
    //         Ic = -Ia - Ib;

    //         *(s16*)&DbgTbl.u16Buf[3] = Ia;
    //     }
    //     // if (a == b[c][4])
    //     {
    //         Ic = Ibus1;
    //         Ib = -Ibus2;
    //         Ia = -Ic - Ib;

    //         *(s16*)&DbgTbl.u16Buf[4] = Ia;
    //     }
    //     // if (a == b[c][5])
    //     {
    //         Ib = Ibus1;
    //         Ic = -Ibus2;
    //         Ia = -Ic - Ib;

    //         *(s16*)&DbgTbl.u16Buf[5] = Ia;
    //     }

    //     switch (ParaTbl.u16Sector)
    //     {
    // #if 1

    //         // case 2:
    //         //     Ia = Ibus1;
    //         //     Ic = -Ibus2;
    //         //     Ib = -Ia - Ic;
    //         //     break;
    //         // case 3:
    //         //     Ic = Ibus1;
    //         //     Ia = -Ibus2;
    //         //     Ib = -Ia - Ic;
    //         //     break;
    //         // case 4:
    //         //     Ib = Ibus1;
    //         //     Ia = -Ibus2;
    //         //     Ic = -Ia - Ib;
    //         //     break;

    //         // case 5:
    //         //     Ia = Ibus1;
    //         //     Ib = -Ibus2;
    //         //     Ic = -Ia - Ib;
    //         //     break;
    //         // case 6:
    //         //     Ic = Ibus1;
    //         //     Ib = -Ibus2;
    //         //     Ia = -Ic - Ib;
    //         //     break;
    //         // case 1:
    //         //     Ib = Ibus1;
    //         //     Ic = -Ibus2;
    //         //     Ia = -Ic - Ib;
    //         //     break;
    // #else
    //         case 6:
    //             Ib = Ibus1;
    //             Ic = -Ibus2;
    //             Ia = -Ic - Ib;
    //             break;
    //         case 5:
    //             Ia = Ibus1;
    //             Ib = -Ibus2;
    //             Ic = -Ia - Ib;
    //             break;
    //         case 4:  // 2
    //             Ia = Ibus1;
    //             Ic = -Ibus2;
    //             Ib = -Ia - Ic;
    //             break;
    //         case 3:  // 3
    //             Ic = Ibus1;
    //             Ia = -Ibus2;
    //             Ib = -Ia - Ic;
    //             break;
    //         case 2:  // 4
    //             Ib = Ibus1;
    //             Ia = -Ibus2;
    //             Ic = -Ia - Ib;
    //             break;
    //         case 1:
    //             Ic = Ibus1;
    //             Ib = -Ibus2;
    //             Ia = -Ic - Ib;
    //             break;

    // #endif
    //         default: break;
    //     }

    // ParaTbl.s16CurPhAFb = Ia;
    // ParaTbl.s16CurPhBFb = Ib;
    // ParaTbl.s16CurPhCFb = Ic;
}

#if 0

typedef struct {
	__IO int16_t s16Vq;
	__IO int16_t s16Vd;
	__IO int16_t s16Valpha;
	__IO int16_t s16Vbeta;
	__IO int16_t s16Ia;
	__IO int16_t s16Ib;
	__IO int16_t s16Ic;
	__IO int16_t s16Ialpha;
	__IO int16_t s16Ibeta;
	__IO int16_t s16Iq;
	__IO int16_t s16Id;	
} FOC_Struct;


void FOC_Coordinate_Transformation(FOC_Struct* FOC, int16_t s16Cos, int16_t s16Sin);

void FOC_Coordinate_Transformation(FOC_Struct* FOC, int16_t s16Cos, int16_t s16Sin)
{
    // a-b-c to alpha-beta (clark)
    FOC->s16Ialpha = FOC->s16Ia;
    FOC->s16Ibeta  = ((int32_t)(FOC->s16Ib - FOC->s16Ic) * 18918) >> 15;

    // alpha-beta to d-q (park)
    FOC->s16Id = ((int32_t)s16Cos * FOC->s16Ialpha + (int32_t)s16Sin * FOC->s16Ibeta) >> 15;
    FOC->s16Iq = (-(int32_t)s16Sin * FOC->s16Ialpha + (int32_t)s16Cos * FOC->s16Ibeta) >> 15;

    // d-q to alpfa-beta (inv park)
    FOC->s16Valpha = ((int32_t)FOC->s16Vd * s16Cos - ((int32_t)FOC->s16Vq * s16Sin)) >> 15;
    FOC->s16Vbeta  = ((int32_t)FOC->s16Vd * s16Sin + ((int32_t)FOC->s16Vq * s16Cos)) >> 15;
}

#endif

void overmod()
{
#if 0
  
    // Modulation
    float V_to_mod = 1.0f / ((2.0f / 3.0f) * v->Umdc);
    float mod_d    = V_to_mod * v->d;
    float mod_q    = V_to_mod * v->q;

    // Vector modulation saturation, lock integrator if saturated
    float mod_scalefactor = 0.80f * M_SQRT3 * 0.5f / sqrtf(mod_d * mod_d + mod_q * mod_q);
    if (mod_scalefactor < 1.0f)
    {
        mod_d *= mod_scalefactor;
        mod_q *= mod_scalefactor;
    }

    v->d = mod_d;
    v->q = mod_q;

#endif
}

f32 NtcConv(u16 advalue)
{
    static RO u16 tbl[98] = {28017, 26826, 25697, 24629, 23618, 22660, 21752, 20892, 20075, 19299, 18560, 18482, 18149, 17632, 16992, 16280, 15535, 14787, 14055, 13354, 12690, 12068, 11490, 10954, 10458, 10000, 9576, 9184, 8819, 8478, 8160, 7861, 7579, 7311, 7056, 6813, 6581, 6357, 6142, 5934, 5734, 5541, 5353, 5173, 4998, 4829, 4665, 4507, 4355, 4208, 4065, 3927, 3794, 3664, 3538, 3415, 3294, 3175, 3058, 2941, 2825, 2776, 2718, 2652, 2582, 2508, 2432, 2356, 2280, 2207, 2135, 2066, 2000, 1938, 1879, 1823, 1770, 1720, 1673, 1628, 1586, 1546, 1508, 1471, 1435, 1401, 1367, 1334, 1301, 1268, 1236, 1204, 1171, 1139, 1107, 1074, 1042, 1010};

    u16 min = 0, max = ARRAY_SIZE(tbl) - 1, mid;  // index

#if 0
    f32 mv  = advalue / 4095.f * 3300.f;
    f32 cmp = mv * 10 / (5 - mv / 1000);
#else
    // 上式的简化
    u16 cmp = 100000000 / (62060606 / advalue - 10000);
#endif

    while ((max - min) > 1)
    {
        // 二分法查表
        mid = (max + min) >> 1;
        (tbl[mid] < cmp) ? (max = mid) : (min = mid);
    }

    f32 ret = min;

    if (max != min)
    {
        ret += (f32)(tbl[min] - cmp) / (f32)(tbl[min] - tbl[max]);
    }

    return ret;
}

extern __IO uint16_t u16IWDGPingPong;
extern __IO uint16_t u16ADC1[8];
uint16_t             u16BEMF_A_B_VoltageDiffAverage;
// extern void DAC12bit_show(int32_t s32DACShowData);
/********************************************************************************************************
**函数信息 ：BEMF_Speed_Detect()  //be executed every 1ms
**功能描述 ：Detect the initial speed and direction by measuring BEMF , be executed every 1ms before motor startup
**输入参数 ：2 phase BEMF
**输出参数 ：BEMF speed and direction
********************************************************************************************************/

uint32_t static u32TempBEMFPhaseA12bit;
uint32_t static u32TempBEMFPhaseB12bit;
uint32_t u32TempBEMFPhaseABDiff12bit;
uint32_t u32TempBEMFComparatorOut;
uint8_t static u8BEMFComparatorOut;

void BEMF_Speed_Detect(BEMF_Speed_Struct* Get_BEMF_Speed, u16 BEMFA_ADC_CHANNEL, u16 BEMFB_ADC_CHANNEL)
{
#define BEMF_MIN_PERIOD_TIME 3  // unit: 1ms, it define the minimum time of BEMF period.

      // if (Get_BEMF_Speed->u16BEMFDetectionTime < BEMF_DETECT_LIMIT_TIME)  // unit : 1ms
    if (1)
    {
        Get_BEMF_Speed->u16BEMFDetectionTime++;  // for control the time duration of BEMF detection

        if (Get_BEMF_Speed->u16BEMFDetectionTime < 50)  // change 10ms to 50ms //20190402
        {
            Get_BEMF_Speed->bBEMFMotorIsRotatingFlag = 0;  // reset this flag
            u16BEMF_A_B_VoltageDiffAverage           = 0;  // reset the difference bemf voltage between phase A,B
        }

        //----- Get phase A,B bemf voltage----------------------------
        u32TempBEMFPhaseA12bit = (2 * u32TempBEMFPhaseA12bit + 1 * BEMFA_ADC_CHANNEL) / 3;  // get the BEMF A voltage //20190413
        u32TempBEMFPhaseB12bit = (2 * u32TempBEMFPhaseB12bit + 1 * BEMFB_ADC_CHANNEL) / 3;  // get the BEMF B voltage //20190413

        //-----detect the motor is standstill or rotating------------
        if (u32TempBEMFPhaseA12bit > u32TempBEMFPhaseB12bit)
        {
            u32TempBEMFPhaseABDiff12bit = u32TempBEMFPhaseA12bit - u32TempBEMFPhaseB12bit;
        }
        else
        {
            u32TempBEMFPhaseABDiff12bit = u32TempBEMFPhaseB12bit - u32TempBEMFPhaseA12bit;
        }

        u16BEMF_A_B_VoltageDiffAverage = u16BEMF_A_B_VoltageDiffAverage + (u32TempBEMFPhaseABDiff12bit >> 2) - (u16BEMF_A_B_VoltageDiffAverage >> 2);
        if (u16BEMF_A_B_VoltageDiffAverage > Get_BEMF_Speed->u16BEMFStandstillThresholdVolt)
        {
            Get_BEMF_Speed->bBEMFMotorIsRotatingFlag = 1;
        }  // confirm the motor is rotating now

        //-----if motor is rotating, then increase EMF_1ms_counter every 1ms, for getting the BEMF speed
        if (Get_BEMF_Speed->bBEMFMotorIsRotatingFlag == 1) { Get_BEMF_Speed->u16BEMF1msCounter++; }  // for detecting rotor speed use only

        //-----use the software to do a comparator function with hystersis------------------------
        u32TempBEMFComparatorOut = u8BEMFComparatorOut;

        if (u32TempBEMFPhaseA12bit > (u32TempBEMFPhaseB12bit + Get_BEMF_Speed->u16BEMFComparatorHystersis))
        {
            u8BEMFComparatorOut = 1;
        }
        else if (u32TempBEMFPhaseB12bit > (u32TempBEMFPhaseA12bit + Get_BEMF_Speed->u16BEMFComparatorHystersis))
        {
            u8BEMFComparatorOut = 0;
        }

        //-----detect the motor's initial speed and direction of rotation-----------------------
        if ((u8BEMFComparatorOut != u32TempBEMFComparatorOut) && (Get_BEMF_Speed->u16BEMF1msCounter > BEMF_MIN_PERIOD_TIME))  // if yes, it means now get the cross point of BEMF A,B
        {                                                                                                                     // LED_TOGGLE();
            if (u8BEMFComparatorOut == 0)                                                                                     // if yes,it means the software comparator output from 1 t 0 (falling edge)
            {
                //---get the motor initial speed--------------------
                Get_BEMF_Speed->u16BEMFSpeed      = ((uint32_t)120 * 1000) / ((uint16_t)Get_BEMF_Speed->u8BEMFPoleNumber * Get_BEMF_Speed->u16BEMF1msCounter);  // 20181108
                Get_BEMF_Speed->u16BEMF1msCounter = 0;                                                                                                          // clear the BEMF time counter

                Get_BEMF_Speed->u16BEMFPhaseABMiddlePoint12bit = (Get_BEMF_Speed->u16BEMFPhaseABMiddlePoint12bit + u32TempBEMFPhaseA12bit) / 2;
                //---get the motor direction by comparing the midpoint potential voltage----------
                if (u32TempBEMFPhaseA12bit > Get_BEMF_Speed->u16BEMFPhaseABMiddlePoint12bit)
                {
                    Get_BEMF_Speed->u8BEMFDirectionFlag = BEMF_DIR_CW;
                }  // the initial direction is CW
                else
                {
                    Get_BEMF_Speed->u8BEMFDirectionFlag = BEMF_DIR_CCW;
                }  // the initial direction is CCW

                if (Get_BEMF_Speed->u16BEMFDetectionTime > (BEMF_DETECT_LIMIT_TIME / 2))
                {
                    Get_BEMF_Speed->u16BEMFDetectionTime = BEMF_DETECT_LIMIT_TIME;
                }  // got real bemf speed, so force out of the BEMF speed detection
            }
            //----get the midpoint potential voltage of phase A,B (add the 1/4 digital filter for average midpoint voltage)---------
            //				u32TempBEMFVoltage12bit = Get_BEMF_Speed->u16BEMFPhaseABMiddlePoint12bit;
            //				u32TempBEMFVoltage12bit = u32TempBEMFVoltage12bit+((u32TempBEMFPhaseA12bit+u32TempBEMFPhaseB12bit)>>3)-(u32TempBEMFVoltage12bit>>2);
            //				Get_BEMF_Speed->u16BEMFPhaseABMiddlePoint12bit = u32TempBEMFVoltage12bit;
            Get_BEMF_Speed->u16BEMFPhaseABMiddlePoint12bit = u32TempBEMFPhaseA12bit;
        }
    }
    else { Get_BEMF_Speed->bBEMFResultValidFlag = 1; }
}
