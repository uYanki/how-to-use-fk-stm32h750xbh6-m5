#ifndef __BUILD_TIME_H__
#define __BUILD_TIME_H__

#include "main.h"

typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
} date_t;

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} time_t;

typedef struct {
    date_t date;
    time_t time;
} date_time_t;

void get_build_time(date_t* date, time_t* time);

#endif
