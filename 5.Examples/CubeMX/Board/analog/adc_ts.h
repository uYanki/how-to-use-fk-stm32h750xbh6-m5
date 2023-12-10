// temperature sensor

float ConvTemp(uint16_t advalue)
{
    uint16_t TS_CAL1 = *(__IO uint16_t*)(0x1FF1E820);
    uint16_t TS_CAL2 = *(__IO uint16_t*)(0x1FF1E840);

    return (110.0 - 30.0) * (advalue - TS_CAL1) / (TS_CAL2 - TS_CAL1) + 30;
}