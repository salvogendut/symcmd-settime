/*
 * HTTP Time Client - Direct system call version
 * Bypasses broken Time_Set C library function
 */

#include <network.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

/* Direct call to Device_TimeSet (#810F) - implemented in timeset_direct.s */
extern void Time_Set_Direct(unsigned char sec, unsigned char min, unsigned char hour,
                            unsigned char day, unsigned char month, unsigned short year,
                            signed char tz);

int main(int argc, char *argv[]) {
    char response[128];
    unsigned long unix_time;
    int result;
    char *ptr;
    unsigned long days, seconds_today;
    unsigned short year;
    unsigned char month, day, hour, minute, second;
    unsigned short days_in_year;
    unsigned short day_of_year;
    TimeConfig config;

    /* Set defaults */
    strcpy(config.server, "http://time.akamai.com/");
    config.timezone = 0;

    printf("HTTP Time Client\n");

    /* Try to load config file */
    Config_Load("settime.cfg", &config);

    if (config.timezone != 0) {
        printf("Timezone: UTC%+d\n", config.timezone);
    }
    printf("\n");

    if (Net_Init() < 0) {
        printf("Error: Network daemon not running\n");
        return 1;
    }

    printf("Fetching time... ");
    memset(response, 0, sizeof(response));
    result = HTTP_GET(config.server, response, sizeof(response) - 1, 0, 0);

    if (result < 0) {
        printf("failed\n");
        return 1;
    }
    printf("OK\n");

    ptr = response;
    while (*ptr == ' ' || *ptr == '\r' || *ptr == '\n' || *ptr == '\t') {
        ptr++;
    }

    unix_time = atol(ptr);
    if (unix_time == 0 || unix_time < 946684800) {
        printf("Error: Invalid timestamp\n");
        return 1;
    }

    /* Apply timezone offset */
    if (config.timezone != 0) {
        long offset = (long)config.timezone * 3600L;
        unix_time = (unsigned long)((long)unix_time + offset);
    }

    /* Calculate time */
    days = unix_time / 86400UL;
    seconds_today = unix_time % 86400UL;

    second = (unsigned char)(seconds_today % 60);
    seconds_today = seconds_today / 60;
    minute = (unsigned char)(seconds_today % 60);
    hour = (unsigned char)(seconds_today / 60);

    /* Calculate date - start from 1970 */
    year = 1970;
    while (1) {
        if ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0))) {
            days_in_year = 366;
        } else {
            days_in_year = 365;
        }
        if (days < days_in_year) break;
        days -= days_in_year;
        year++;
    }

    /* Calculate month and day */
    day_of_year = (unsigned short)days;
    month = 1;
    while (month <= 12) {
        unsigned char days_this_month;
        if (month == 2) {
            if ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0))) {
                days_this_month = 29;
            } else {
                days_this_month = 28;
            }
        } else if (month == 4 || month == 6 || month == 9 || month == 11) {
            days_this_month = 30;
        } else {
            days_this_month = 31;
        }
        if (day_of_year < days_this_month) break;
        day_of_year -= days_this_month;
        month++;
    }
    day = (unsigned char)(day_of_year + 1);

    if (config.timezone != 0) {
        printf("Setting: %04d-%02d-%02d %02d:%02d:%02d (UTC%+d)\n",
               year, month, day, hour, minute, second, config.timezone);
    } else {
        printf("Setting: %04d-%02d-%02d %02d:%02d:%02d (UTC)\n",
               year, month, day, hour, minute, second);
    }

    /* Call system function directly
     * Note: We already adjusted the time by timezone offset above,
     * so we pass 0 to indicate the time is already in local time */
    Time_Set_Direct(second, minute, hour, day, month, year, 0);

    printf("System clock updated!\n");

    return 0;
}
