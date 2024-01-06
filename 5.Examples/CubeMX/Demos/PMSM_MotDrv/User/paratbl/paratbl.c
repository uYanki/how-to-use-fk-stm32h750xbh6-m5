#include "paratbl.h"
#include "system/buildtime.h"

uint8_t u8ParaBuf[sizeof(ParaTable_t)] = {0};
uint8_t u8DbgBuf[sizeof(DebugTable_t)] = {0};

void ParaTbl_Init(void)
{
    date_time_t buildTime;
    get_build_time(&(buildTime.date), &(buildTime.time));

    ParaTbl.u32McuSwVer = PROJECT_VERSION;

    ParaTbl.u32SwBuildDate = buildTime.date.year * 1000 + buildTime.date.month * 100 + buildTime.date.day;
    ParaTbl.u32SwBuildDate = buildTime.time.hour * 1000 + buildTime.time.minute * 100 + buildTime.time.second;
}
