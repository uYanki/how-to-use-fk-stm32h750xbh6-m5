# [霍尔信号、编码器信号与电机转向](https://www.cnblogs.com/smilingfrog/p/7771367.html)

从电机出轴方向看去，电机轴逆时针转动，霍尔信号的序列为

![img](.assest/ST_Hall/484857-20171102094302123-1029611777.png)

编码器信号的序列为

![img](.assest/ST_Hall/484857-20171102094655420-70829828.png)

将霍尔信号按照H3 H2 H1的顺序组成三位二进制数，则霍尔信号翻译成状态为

![img](.assest/ST_Hall/484857-20171102094946513-1949849365.png)

以120°放置霍尔为例

如不给电机加电，使用示波器测量三个霍尔信号和电机三相反电动势，

按照上面所说的方向用手转动电机得到下图

![img](.assest/ST_Hall/484857-20171102095325029-548477253.png)

① H1的上升沿对应电机q轴与H1位置电角度夹角为0°，也就是state4->state5的变化点；

② H3的下降沿对应电机q轴与H1位置电角度夹角为60°，也就是state5->state1的变化点；

③ H2的上升沿对应电机q轴与H1位置电角度夹角为120°，也就是state1>state3的变化点；

④ H1的下降沿对应电机q轴与H1位置电角度夹角为180°，也就是state3->state2的变化点；

⑤ H3的上升沿对应电机q轴与H1位置电角度夹角为240°，也就是state2->state6的变化点；

⑥ H3的下降沿对应电机q轴与H1位置电角度夹角为300°，也就是state6->state4的变化点；

d轴永远滞后q轴90°。

电机按上述转向转动，当q轴与电机某相的电角度夹角为0°时，这时磁场由S极切换为N极，磁场强度为0，反电动势从0开始

当q轴与电机某相的电角度夹角为90°时，该相反电动势电压在最高点，夹角为180°时，反电动势电压在最低点。

由上图可以看出q轴与H1位置电角度夹角为0后，继续转动90°电角度后A相反电动势达到最高点，

也就是说①时，q轴与A相电角度夹角为0°，d轴与A相电角度夹角为-90°。

霍尔安装位置与线圈中心重合。

在电机制造中，对给定的H1位置与A相电角度夹角，霍尔实际安装的机械偏角由极对数和电子偏角共同决定。







#### 霍尔状态读取

```c
void HallEnc_ReadState(hall_encoder_t* pEnc)
{
    uint8_t HallStateCur = 0, HallStatePrev;

    do {
        HallStatePrev = HallStateCur;
        HallStateCur  = 0;

        switch (pEnc->Placement)
        {
            case 60:  // DEGREES_60
            {
                HallStateCur |= ((HAL_GPIO_ReadPin(pEnc->HB_Port, pEnc->HB_Pin) == GPIO_PIN_SET) ^ 1) << 2;
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HC_Port, pEnc->HC_Pin) == GPIO_PIN_SET) << 1;
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HA_Port, pEnc->HA_Pin) == GPIO_PIN_SET);
                break;
            }
            case 120:  // DEGREES_120
            {
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HC_Port, pEnc->HC_Pin) == GPIO_PIN_SET) << 2;
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HB_Port, pEnc->HB_Pin) == GPIO_PIN_SET) << 1;
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HA_Port, pEnc->HA_Pin) == GPIO_PIN_SET);
                break;
            }
        }

    } while (HallStateCur != HallStatePrev);

    pEnc->HallState = HallStateCur;
}

```

#### 霍尔测速

① 方式1：对单路霍尔的进行边沿计数

![image-20240106171520475](.assest/ST_Hall/image-20240106171520475.png)

```c
// 原理：转子每转1圈，每路霍尔会产生与极对数相等个脉冲

int main()
{
    HAL_TIM_Base_Start(&htim);
    
    while(1)
    {
        SpeedCalc();
    }
}

static tick_t tSpdCalc = 0;

void SpeedCalc(void)
{
    HallEnc_ReadState(); // 更新电角度
    
   if (DelayNonBlockMS(tSpdCalc, 1000))
    {
        s16SpdFb = 60.f * __HAL_TIM_GET_COUNTER(&htim) / u16MotPolePairs; // RPM
        __HAL_TIM_SET_COUNTER(&htim, 0); // 复位计数值
        tSpdCalc = DelayNonGetTick();
    }
}
```

② 方式2：霍尔传感器模式 + 中断

![image-20240106172710249](.assest/ST_Hall/image-20240106172710249.png)

![image-20240106172719536](.assest/ST_Hall/image-20240106172719536.png)

![image-20240106172803221](.assest/ST_Hall/image-20240106172803221.png)

```c
// 原理：转子每转1圈，3路霍尔会产生6*极对数个脉冲

int main()
{
    HAL_TIM_Base_Start_IT(&htim);
    
    while(1)
    {
        SpeedCalc();
    }
}

static tick_t tSpdCalc = 0;

void SpeedCalc(void)
{
   if (DelayNonBlockMS(tSpdCalc, 1000))
    {
        s16SpdFb = 60.f * u16HallEdgeCnt / u16MotPolePairs / 6; // RPM
        u16HallEdgeCnt = 0; // 复位计数值
        tSpdCalc = DelayNonGetTick();
    }
}

void TIM8_UP_TIM13_IRQHandler(void) // 更新电角度
{
	if (__HAL_TIM_GET_FLAG(&htim8, TIM_IT_TRIGGER))
    {
        __HAL_TIM_CLEAR_FLAG(&htim8, TIM_IT_TRIGGER);
        HallEnc_ReadState();
        u16HallEdgeCnt++;
    }
}
```

注：不要用以下中断

```
HAL_TIMEx_HallSensor_Start_IT() 
HAL_TIMEx_HallSensor_GetState() // 实际取的是定时器状态, 不是霍尔状态
```

