/*
 * Configuration file support for settime
 */

#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char server[64];
    signed char timezone;
} TimeConfig;

/* Load config from file, returns 0 on success, -1 if not found */
int Config_Load(const char* filename, TimeConfig* config);

#endif
