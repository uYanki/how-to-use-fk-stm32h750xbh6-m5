#include "svpwm.h"

/**
 * @brief Park Transformation
 * @param [in] alpha, beta
 * @param [out] d, q
 */
void park(foc_t* v)
{
    v->d = v->beta * v->sin + v->alpha * v->cos;
    v->q = v->beta * v->cos - v->alpha * v->sin;
}

/**
 * @brief Inverse Park Transformation
 * @param [in] d, q
 * @param [out] alpha, beta
 */
void ipark(foc_t* v)
{
    v->alpha = v->d * v->cos - v->q * v->sin;
    v->beta  = v->d * v->sin + v->q * v->cos;
}

/**
 * @brief Clarke Transformation
 * @param [in] phase_a, phase_b, phase_c
 * @param [out] alpha, beta
 */
void clarke(foc_t* v)
{
#if 0  // 2-phase
    v->alpha = v->phase_a;
    v->beta  = (v->phase_a + v->phase_b * 2) * M_INVSQRT3;
#else  // 3-phase
    v->alpha = v->phase_a;
    v->beta  = (v->phase_b - v->phase_c) * M_INVSQRT3;
#endif
}

/**
 * @brief Inverse Clarke Transformation
 * @param [in] alpha, beta
 * @param [out] phase_a, phase_b, phase_c
 */
void iclarke(foc_t* v)
{
    v->phase_a = v->alpha;
    v->phase_b = +0.5f * (M_SQRT3 * v->beta - v->alpha);
    v->phase_c = -0.5f * (M_SQRT3 * v->beta + v->alpha);
}

/**
 * @brief Space Vector PWM Generator
 *
 * @param [in] alpha, beta
 * @param [out] Ta, Tb, Tc, sector
 */
void svpwm(foc_t* v)
{
    float32_t X = v->beta;
    float32_t Y = (v->beta + M_SQRT3 * v->alpha) * 0.5f;
    float32_t Z = Y - X;

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
}

void zero_inject(foc_t* v)
{
    float32_t V0 = -0.5 * (v->phase_min + v->phase_max);

    v->Ta = v->phase_a + V0;
    v->Tb = v->phase_b + V0;
    v->Tc = v->phase_c + V0;
}

void ph_order(foc_t* v)
{
    if (v->phase_a >= v->phase_b)
    {
        if (v->phase_a >= v->phase_c)
        {
            if (v->phase_b >= v->phase_c)
            {
                v->sector    = SECTOR_1;
                v->phase_max = v->phase_a;
                v->phase_mid = v->phase_b;
                v->phase_min = v->phase_c;

                return;
            }
            else
            {
                v->sector    = SECTOR_6;
                v->phase_max = v->phase_a;
                v->phase_mid = v->phase_c;
                v->phase_min = v->phase_b;

                return;
            }
        }
        else
        {
            v->sector    = SECTOR_5;
            v->phase_max = v->phase_c;
            v->phase_mid = v->phase_a;
            v->phase_min = v->phase_b;

            return;
        }
    }
    else
    {
        if (v->phase_a < v->phase_c)
        {
            if (v->phase_b < v->phase_c)
            {
                v->sector    = SECTOR_4;
                v->phase_max = v->phase_c;
                v->phase_mid = v->phase_b;
                v->phase_min = v->phase_a;

                return;
            }
            else
            {
                v->sector    = SECTOR_3;
                v->phase_max = v->phase_b;
                v->phase_mid = v->phase_c;
                v->phase_min = v->phase_a;

                return;
            }
        }
        else
        {
            v->sector    = SECTOR_2;
            v->phase_max = v->phase_b;
            v->phase_mid = v->phase_a;
            v->phase_min = v->phase_c;

            return;
        }
    }
}

/**
 *  Ia = Im * cos(θ)
 *  Ib = Im * cos(θ-2π/3)
 *  Ic = Im * cos(θ-4π/3)
 *     = Im * cos(θ+2π/3)
 *
 *  Ia + Ib + Ic = 0 (基尔霍夫定律)
 *
 */
