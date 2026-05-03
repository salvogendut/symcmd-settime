/*
 * Configuration file parser for settime
 * File format:
 *   server=time.akamai.com
 *   timezone=+2
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

int Config_Load(const char* filename, TimeConfig* config) {
    FILE* f;
    char line[128];
    char* equals;
    char* value;
    char* key;
    char* end;
    int found = 0;

    f = fopen(filename, "r");
    if (!f) {
        return -1;
    }

    while (fgets(line, sizeof(line), f)) {
        /* Remove newline */
        line[strcspn(line, "\r\n")] = 0;

        /* Skip empty lines and comments */
        if (line[0] == 0 || line[0] == '#' || line[0] == ';') {
            continue;
        }

        /* Find the = sign */
        equals = strchr(line, '=');
        if (!equals) {
            continue;
        }

        *equals = 0;
        key = line;
        value = equals + 1;

        /* Trim trailing spaces from key */
        end = equals - 1;
        while (end >= key && (*end == ' ' || *end == '\t')) {
            *end = 0;
            end--;
        }

        /* Trim leading spaces from value */
        while (*value == ' ' || *value == '\t') {
            value++;
        }

        /* Trim trailing spaces from value */
        end = value + strlen(value) - 1;
        while (end >= value && (*end == ' ' || *end == '\t')) {
            *end = 0;
            end--;
        }

        /* Parse server */
        if (strcmp(key, "server") == 0) {
            /* Add http:// prefix if not present */
            if (strncmp(value, "http://", 7) == 0) {
                strncpy(config->server, value, sizeof(config->server) - 1);
            } else {
                snprintf(config->server, sizeof(config->server), "http://%s/", value);
            }
            config->server[sizeof(config->server) - 1] = 0;
            found = 1;
        }
        /* Parse timezone */
        else if (strcmp(key, "timezone") == 0) {
            char* tz_value = value;
            /* Skip leading + sign if present (atoi may not handle it) */
            if (*tz_value == '+') {
                tz_value++;
            }
            config->timezone = (signed char)atoi(tz_value);
            found = 1;
        }
    }

    fclose(f);
    return found ? 0 : -1;
}
