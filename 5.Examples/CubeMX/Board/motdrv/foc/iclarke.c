// #include "iclarke.h"

// #include "main.h"
// #include <stdbool.h>

// #define M_PI                   (3.14159265358f)
// #define M_2PI                  (6.28318530716f)
// #define SQRT3                  (1.73205080757f)
// #define SQRT3_BY_2             (0.86602540378f)
// #define ONE_BY_SQRT3           (0.57735026919f)
// #define TWO_BY_SQRT3           (1.15470053838f)

// #define SQ(x)                  ((x) * (x))
// #define ABS(x)                 ((x) > 0 ? (x) : -(x))
// #define MIN(a, b)              (((a) < (b)) ? (a) : (b))
// #define MAX(a, b)              (((a) > (b)) ? (a) : (b))
// #define CLAMP(x, lower, upper) (MIN(upper, MAX(x, lower)))

// // Return the sign of the argument. -1 if negative, 1 if zero or positive.
// #define SIGN(x)                ((x < 0) ? -1 : 1)

// typedef struct {
//     uint16_t adc_vbus;
//     uint16_t adc_phase_a, adc_phase_b, adc_phase_c;
//     float    v_bus;               // DC link voltage
//     float    i_a, i_b, i_c;       // Phase currents
//     float    i_a_filt, i_b_filt;  // Phase currents
//     float    dtc_a, dtc_b, dtc_c;
//     float    i_d_filt, i_q_filt, i_bus_filt;  // D/Q currents
//     float    sin, cos;
//     float    current_ctrl_integral_d, current_ctrl_integral_q;  // Current error integrals
//     int      adc_phase_a_offset;
//     int      adc_phase_b_offset;
// } FOCStruct;

// extern FOCStruct Foc;

// FOCStruct Foc;

// void apply_voltage_timings(float vbus, float v_d, float v_q, float pwm_phase)
// {
//     // Modulation
//     float V_to_mod = 1.0f / ((2.0f / 3.0f) * vbus);
//     float mod_d    = V_to_mod * v_d;
//     float mod_q    = V_to_mod * v_q;

//     // Vector modulation saturation, lock integrator if saturated
//     float mod_scalefactor = 0.80f * SQRT3_BY_2 * 1.0f / sqrtf(mod_d * mod_d + mod_q * mod_q);
//     if (mod_scalefactor < 1.0f)
//     {
//         mod_d *= mod_scalefactor;
//         mod_q *= mod_scalefactor;
//     }

//     // Inverse park transform
//     float mod_alpha;
//     float mod_beta;
//     fast_sincos(pwm_phase, &Foc.sin, &Foc.cos);
//     inverse_park(mod_d, mod_q, &mod_alpha, &mod_beta);

//     // SVM
//     svm(mod_alpha, mod_beta, &Foc.dtc_a, &Foc.dtc_b, &Foc.dtc_c);

//     // Apply duty
//     TIMER_CH0CV(TIMER0) = (uint16_t)(Foc.dtc_a * (float)PWM_ARR);
//     TIMER_CH1CV(TIMER0) = (uint16_t)(Foc.dtc_b * (float)PWM_ARR);
//     TIMER_CH2CV(TIMER0) = (uint16_t)(Foc.dtc_c * (float)PWM_ARR);
// }

// void FOC_current(FOCStruct* foc, float Id_des, float Iq_des, float I_phase, float pwm_phase)
// {
//     // Clarke transform
//     float i_alpha, i_beta;
//     clarke_transform(foc->i_a_filt, foc->i_b_filt, foc->i_c, &i_alpha, &i_beta);

//     // Park transform
//     float i_d, i_q;
//     fast_sincos(I_phase, &Foc.sin, &Foc.cos);
//     park_transform(i_alpha, i_beta, &i_d, &i_q);

//     // Current Filter used for report
//     foc->i_d_filt = 0.95f * foc->i_d_filt + 0.05f * i_d;
//     foc->i_q_filt = 0.95f * foc->i_q_filt + 0.05f * i_q;

//     // Apply PI control
//     float Ierr_d = Id_des - i_d;
//     float Ierr_q = Iq_des - i_q;
//     float v_d    = UsrConfig.current_iq_kp * Ierr_d + foc->current_ctrl_integral_d;
//     float v_q    = UsrConfig.current_iq_kp * Ierr_q + foc->current_ctrl_integral_q;

//     // Modulation
//     float mod_to_V = (2.0f / 3.0f) * foc->v_bus;

//     float mod_d = (1.0f / mod_to_V) * v_d;
//     float mod_q = (1.0f / mod_to_V) * v_q;

//     // Vector modulation saturation, lock integrator if saturated
//     float mod_scalefactor = 0.8f * SQRT3_BY_2 * 1.0f / sqrtf(mod_d * mod_d + mod_q * mod_q);
//     if (mod_scalefactor < 1.0f)
//     {
//         mod_d *= mod_scalefactor;
//         mod_q *= mod_scalefactor;
//         foc->current_ctrl_integral_d *= 0.99f;
//         foc->current_ctrl_integral_q *= 0.99f;
//     }
//     else
//     {
//         foc->current_ctrl_integral_d += Ierr_d * (UsrConfig.current_iq_ki * DT);
//         foc->current_ctrl_integral_q += Ierr_q * (UsrConfig.current_iq_ki * DT);
//     }

//     //	// Compute estimated bus current
//     //    foc->i_bus_filt = mod_d * foc->i_d_filt + mod_q * foc->i_q_filt;

//     // Inverse park transform
//     float mod_alpha;
//     float mod_beta;
//     inverse_park(mod_d, mod_q, &mod_alpha, &mod_beta);

//     // SVM
//     float dtc_a, dtc_b, dtc_c;
//     svm(mod_alpha, mod_beta, &dtc_a, &dtc_b, &dtc_c);

//     // Apply duty
//     TIM0_REGISTER_P->CCR1 = (uint16_t)(dtc_a * (float)PWM_ARR);
//     TIM0_REGISTER_P->CCR2 = (uint16_t)(dtc_b * (float)PWM_ARR);
//     TIM0_REGISTER_P->CCR3 = (uint16_t)(dtc_c * (float)PWM_ARR);
// }
