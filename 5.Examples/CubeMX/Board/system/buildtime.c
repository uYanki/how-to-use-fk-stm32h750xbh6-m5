#include "buildtime.h"
#include <string.h>

static const char* m_data = __DATE__;
static const char* m_time = __TIME__;

void get_build_time(date_t* date, time_t* time)
{
    if (date != nullptr)
    {
        const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

        uint8_t month = 0;

        while (strcmp(m_data, months[month++]) != m_data[3])
            ;

        date->month = month;

        date->day = (m_data[5] - '0');

        if (m_data[4] != ' ')
        {
            date->day += (m_data[4] - '0') * 10;
        }

        date->year = ((m_data[7] - '0') * 1000 + (m_data[8] - '0') * 100 + (m_data[9] - '0') * 10 + (m_data[10] - '0'));
    }

    if (time != nullptr)
    {
        time->hour   = (m_time[0] - '0') * 10 + (m_time[1] - '0');
        time->minute = (m_time[3] - '0') * 10 + (m_time[4] - '0');
        time->second = (m_time[6] - '0') * 10 + (m_time[7] - '0');
    }
}
