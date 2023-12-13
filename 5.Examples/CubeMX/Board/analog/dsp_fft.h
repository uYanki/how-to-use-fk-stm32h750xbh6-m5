#ifndef __DSP_FFT_H__
#define __DSP_FFT_H__

#include <arm_math.h>
#include <stdbool.h>

#define RAD2DEG (180.0 / 3.1415926)

float32_t round_n(float32_t value, uint8_t n);

bool FFT(float32_t* aInput, uint32_t N, uint32_t Fs_Hz);

#endif
