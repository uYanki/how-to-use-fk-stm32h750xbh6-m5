#include "pid.h"
#include <string.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
//

void PID_PostHandler(PID_t* p, f32 out_p)
{
#if 1

    // 幅度限制

    if (p->out > p->hi)
    {
        p->out = p->hi;
    }
    else if (p->out < p->lo)
    {
        p->out = p->lo;
    }

#endif

#if 1

    // 斜率限制

    if (p->ramp > 0)
    {
        f32 rate = (p->out - out_p) / p->Ts;

        if (rate > p->ramp)
        {
            p->out = out_p + p->ramp * p->Ts;
        }
        else if (rate < -p->ramp)
        {
            p->out = out_p - p->ramp * p->Ts;
        }
    }

#endif
}

void PID_Handler_Base(PID_t* p)  // 基本式 or 位置式
{
    f32 err_p = p->err;  //!< previous error
    f32 out_p = p->out;  //!< previous output value

    p->err = p->ref - p->fbk;

    p->prop = p->err;
    p->inte += p->err * p->Ts;
    p->deri = (p->err - err_p) / p->Ts;

    p->out = p->Kp * p->prop + p->Ki * p->inte + p->Kd * p->deri;

    PID_PostHandler(p, out_p);
}

void PID_Handler_Tustin(PID_t* p)  // Tustin 散点积分
{
    f32 err_p = p->err;  //!< previous error
    f32 out_p = p->out;  //!< previous output value

    p->err = p->ref - p->fbk;

    p->prop = p->err;
    p->inte = 0.5f * (p->err + err_p) * p->Ts;
    p->deri = (p->err - err_p) / p->Ts;

    p->out += p->Kp * p->prop + p->Ki * p->inte + p->Kd * p->deri;

    PID_PostHandler(p, out_p);
}

void PID_Handler_Inc(PID_t* p)  // 增量式
{
    // Δ(k) = kp (e(k) - e(k-1)) + ki e(k) + kd (e(k) - 2e(k-1) + e(k-2))

    if (p->args == nullptr)
    {
        p->args = malloc(sizeof(f32) * 1);
    }

    f32 err_p = p->err;  //!< previous error
    f32 out_p = p->out;  //!< previous output value

    f32* err_pp = (f32*)(p->args);  //!< previous previous error

    p->err = p->ref - p->fbk;

    p->prop = (p->err - err_p);
    p->inte = p->err * p->Ts;
    p->deri = (p->err - 2 * err_p + *err_pp) / p->Ts;

    p->out += p->Kp * p->prop + p->Ki * p->inte + p->Kd * p->deri;

    *err_pp = err_p;

    PID_PostHandler(p, out_p);
}

void PID_Handler_Blance(PID_t* p)  // 平衡式
{
    f32 err_p = p->err;  //!< previous error
    f32 out_p = p->out;  //!< previous output value

    p->err = p->ref - p->fbk;

    p->prop = p->err;
    p->inte += (p->err + err_p) * p->Ts / 2;
    p->deri = (p->err - err_p) / p->Ts;

    p->out = p->Kp * p->prop + p->Ki * p->inte + p->Kd * p->deri;

    PID_PostHandler(p, out_p);
}

void PID_Deinit(PID_t* p)
{
    if (p->args != nullptr)
    {
        free(p->args);
    }
}

//-----------------------------------------------------------------------------
//

#if 0

#include <stdio.h>

int main()
{
    // 使用 curve_viewer 观察曲线变化

    PID_Handler_t cb[] = {
        &PID_Handler_Base,
        &PID_Handler_Tustin,
        &PID_Handler_Inc,
        &PID_Handler_Blance,
    };

    PID_t PID[] = {
        {.Kp = 0.1, .Ki = 0.2, .Kd = 0.0, .ref = 10000, .ramp = 0, .lo = 0, .hi = 2000, .Ts = 1},
        {.Kp = 0.1, .Ki = 0.2, .Kd = 0.0, .ref = 10000, .ramp = 0, .lo = 0, .hi = 2000, .Ts = 1},
        {.Kp = 0.1, .Ki = 0.2, .Kd = 0.0, .ref = 10000, .ramp = 0, .lo = 0, .hi = 2000, .Ts = 1},
        {.Kp = 0.1, .Ki = 0.2, .Kd = 0.0, .ref = 10000, .ramp = 0, .lo = 0, .hi = 2000, .Ts = 1},
    };

    FILE* f = fopen("dat.csv", "w+");

    if (f)
    {
        int i = 0, n = 100;

        while (i < ARRAY_SIZE(PID))  // 编号
        {
            fprintf(f, "%d%c", i, ++i == ARRAY_SIZE(PID) ? '\n' : ',');
        }

        i = 0;
        while (i < ARRAY_SIZE(PID))  // 起始点
        {
            fprintf(f, "%.2f%c", i, ++i == ARRAY_SIZE(PID) ? '\n' : ',');
        }

        while (n--)  // 迭代次数
        {
            i = 0;
            while (i < ARRAY_SIZE(PID))
            {
                PID[i].fbk = PID[i].out;
                (cb[i])(&PID[i]);
                fprintf(f, "%.2f", PID[i].out);
                fprintf(f, "%c", ++i == ARRAY_SIZE(PID) ? '\n' : ',');
            }
        }

        fclose(f);
    }

    return 0;
}

#endif