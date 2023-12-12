
#include <arm_math.h>

#define RAD2DEG (180.0 / 3.1415926)
extern arm_status arm_atan2_f32(float32_t y, float32_t x, float32_t* result);

/**
 * @brief 四舍五入, 保留n位小数
 */
float32_t round_n(float32_t value, uint8_t n)
{
    float32_t coeff = 1.f;

    while (n--)
    {
        coeff *= 10.f;
    }

    return round(value * coeff) / coeff;
}

typedef struct {
    float32_t* aInput;  // nSize
    float32_t* aFFT;    // nSize*2
    float32_t* aMag;    // nSize
    float32_t* aPhase;  // nSize

    uint32_t nSize;  // 采样点数
    uint32_t Fs_hz;  // 采样频率
} fft_t;

//-------------------------------------------------------------------------
// 求频谱

/**
 * @brief FFT
 *
 * @param ifftFlag 0:正变换, 1:逆变换
 *
 * @note 输入输出
 *
 *    INPUT:  原始数据
 *    OUTPUT: 双边频谱
 *
 *      直流分量: 频谱数组中下标为0的元素。
 *      交流分量: 频谱数组中下标非0的元素。
 *
 * @note 频谱数组中的元素是幅度，而不是幅值
 *
 *       直流分量幅值 = 直流分量幅度 / 采样点数
 *       交流分量幅值 = 交流分量幅度 / (采样点数/2)
 *
 */

void cfft(fft_t* p)
{
    float32_t* src = &p->aInput[0];
    float32_t* dst = &p->aFFT[0];

    for (uint32_t i = 0; i < p->nSize; ++i)
    {
        *dst++ = *src++;
        *dst++ = 0.f;
    }

    arm_cfft_instance_f32 S;
    arm_cfft_init_f32(&S, p->nSize);
    arm_cfft_f32(&S, p->aFFT, 0, 1);
}

void rfft(fft_t* p)
{
    arm_rfft_fast_instance_f32 S;
    arm_rfft_fast_init_f32(&S, p->nSize);
    arm_rfft_f32(&S, p->aInput, p->aFFT, 0);
}

//-------------------------------------------------------------------------
// 求幅值

/**
 * 幅频响应: X轴为频率, Y轴为幅值
 */

void fft_calc_mag(fft_t* p)
{
    arm_cmplx_mag_f32(p->aFFT, p->aMag, p->nSize);  // 求模

    p->aMag[0] /= p->nSize;

    for (i = 1; i < p->nSize; i++)
    {
        p->aMag[i] /= p->nSize / 2;
    }
}

//-------------------------------------------------------------------------
// 求相位

/**
 * 相频响应: X轴为频率, Y轴为相位
 */

void fft_calc_phase(fft_t* p)
{
    float64_t  x, y;
    float32_t  phase;
    float32_t* p = &p->aFFT[0];

    for (i = 0; i < p->nSize; i++)
    {
        x = *p++;
        y = *p++;

        arm_atan2_f32(y, x, &phase);

        if (0.5 > p->aMag[i])
        {
            p->aPhase[i] = 0;
        }
        else
        {
            // 弧度转角度
            p->aPhase[i] = phase * RAD2DEG;
        }
    }
}

//-------------------------------------------------------------------------
// 求频率

float32_t fft_calc_freq(fft_t* p, uint32_t index)
{
    /**
     * 分辨率 = 采样频率/采样点数, 即频谱中每两个相邻频率之间的频率间隔
     * 实际频率 = 数组下标 * 分辨率 = 采样频率 * 数组下标 / 采样点数
     */

    return p->Fs_hz * index / p->nSize;
}

void fft_sig_disp(fft_t* p)  // 信号成分显示
{
#if 0

    /* 获取主成分的频率 */

    uint32_t  MaxFreqIdx;  // 最大频率对应下标
    float32_t MaxFreqVal;  // 最大频率对应幅值

    // aFFT[0] 是直流分量, 忽略掉
    arm_max_f32(&aMag[1], p->nSize - 1, &MaxFreqVal, &MaxFreqIdx);

    float32_t MaxFreq = (float32_t)p->s_Hz * MaxFreqIdx / p->nSize;

#endif

    // FFT 输出的是双边频谱, 这里取前半部分的单边频谱即可

    for (i = 0; i < (p->nSize / 2); i++)
    {
        if (p->aMag[i] > 0.1)
        {
            printf("%f, %f, %f\n", p->aMag[i], round_n(p->aPhase[i], 4), fft_calc_freq(p, i));
        }
    }
}

//-------------------------------------------------------------------------
//

void fft_res_disp(fft_t* p)
{
    // 推荐使用 vofa+ 来查看输出结果

    for (i = 0; i < p->nSize; i++)
    {
        printf("%f, %f, %f, %f\n", p->aFFT[i], p->aMag[i], p->aPhase[i], fft_calc_freq(p, i));
    }
}

//

void ADC2_FFT_Proc(uint16_t* src)
{
    static float32_t aFFT[p->nSize * 2] = {0};
    static float32_t aMag[p->nSize]     = {0};
    static float32_t aPhase[p->nSize]   = {0};

    //-------------------------------------------------------------------------

    uint16_t i;

#if 1

    float32_t* dest = &aFFT[0];

    for (i = 0; i < p->nSize; ++i)
    {
#if 1
        // 50Hz cos, fs = 1024, phase = 60
        *dest++ = 1.2 + 2.5 * cos(2 * 3.1415926 * 50 * i / 1024 + 3.1415926 / 3) + 1.9 * cos(2 * 3.1415926 * 100 * i / 1024 + 3.1415926 / 4);
        *dest++ = 0;
#else
        *dest++ = *src++;
        *dest++ = 0;
#endif
    }

#endif

#if 0

#endif

    while (1)
    {
    }
}
