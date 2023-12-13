
#include "dsp_fft.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

#define BUFSRC 0

float32_t* BUFF[8000] = {0};

bool FFT(float32_t* aInput, uint32_t N, uint32_t Fs_Hz)
{
    bool result = false;

    uint32_t i;

#if BUFSRC
    float32_t* aFFT   = (float32_t*)malloc(sizeof(float32_t) * N * 2);
    float32_t* aMag   = (float32_t*)malloc(sizeof(float32_t) * N);
    float32_t* aPhase = (float32_t*)malloc(sizeof(float32_t) * N);
#else
    float32_t* aFFT   = BUFF;
    float32_t* aMag   = aFFT + N * 2;
    float32_t* aPhase = aMag + N;
#endif

    if (aFFT == 0 || aMag == 0 || aPhase == 0)
    {
        goto exit;
    }

    float32_t *src, *dst;

    //-------------------------------------------------------------------------
    // 求频谱

#if 1

    arm_rfft_fast_instance_f32 S;
    arm_rfft_fast_init_f32(&S, N);
    arm_rfft_fast_f32(&S, aInput, aFFT, 0);

#else

    src = &aInput[0];
    dst = &aFFT[0];

    for (uint32_t i = 0; i < N; ++i)
    {
        *dst++ = *src++;  // 实部
        *dst++ = 0.f;     // 虚部
    }

    arm_cfft_instance_f32 S;
    arm_cfft_init_f32(&S, N);
    arm_cfft_f32(&S, aFFT, 0, 1);

#endif

    //-------------------------------------------------------------------------
    // 求幅值, 幅频响应(X轴为频率, Y轴为幅值)

    arm_cmplx_mag_f32(aFFT, aMag, N);  // 求模

    aMag[0] /= N;

    for (i = 1; i < N; i++)
    {
        aMag[i] /= N / 2;
    }

    //-------------------------------------------------------------------------
    // 求相位, 相频响应(X轴为频率, Y轴为相位)

    float32_t x, y;
    float32_t phase;

    src = &aFFT[0];

    for (i = 0; i < N; i++)
    {
        x = *src++;
        y = *src++;

        arm_atan2_f32(y, x, &phase);

        if (0.5 > aMag[i])
        {
            aPhase[i] = 0;
        }
        else
        {
            // 弧度转角度
            aPhase[i] = phase * RAD2DEG;
        }
    }

#if 1

    //-------------------------------------------------------------------------
    // 显示主成分

    uint32_t  MaxMagIdx;  // 交流分量最大幅值对应下标
    float32_t MaxMagVal;  // 交流分量最大幅值对应幅值

    // aFFT[0] 是直流分量, 忽略掉
    arm_max_f32(&aMag[1], N - 1, &MaxMagVal, &MaxMagIdx);

    MaxMagIdx += 1;

    float32_t MainFreq = (float32_t)Fs_Hz * MaxMagIdx / N;

    // 幅值, 相位, 频率
    printf("%f, %f, %f\n", aMag[MaxMagIdx], round_n(aPhase[MaxMagIdx], 4), MainFreq);

#endif

#if 0

    //-------------------------------------------------------------------------
    // 显示信号成分, FFT 输出的是双边频谱, 这里取前半部分的单边频谱即可

    /**
     * 分辨率 = 采样频率/采样点数, 即频谱中每两个相邻频率之间的频率间隔
     * 实际频率 = 数组下标 * 分辨率 = 采样频率 * 数组下标 / 采样点数
     */

    printf("----------------------------------------\n");

    for (i = 0; i < (N / 2); i++)
    {
        if (aMag[i] > 0.1)
        {
            // 幅值, 相位, 频率
            printf("%f, %f, %f\n", aMag[i], round_n(aPhase[i], 4), (float32_t)Fs_Hz * i / N);
        }
    }

#endif

#if 0

    //-------------------------------------------------------------------------
    // 显示所有结果

    printf("----------------------------------------\n");

    for (i = 0; i < N; i++)
    {
        // 推荐使用 vofa+ 来查看输出结果

        printf("%f, %f, %f, %f\n", aFFT[i], aMag[i], aPhase[i], (float32_t)Fs_Hz * i / N);
    }

#endif

    result = true;

    //-------------------------------------------------------------------------

exit:

#if BUFSRC
    free(aFFT);
    free(aMag);
    free(aPhase);
#endif

    return result;
}
