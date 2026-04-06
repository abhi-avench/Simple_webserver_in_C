/**
 * @file    logger.c
 * @brief   Simple logging implementation
 *
 * @note    INTENTIONAL MISRA-C:2012 VIOLATIONS (for educational purposes):
 *          See comments marked [MISRA VIOLATION] throughout this file.
 *
 * @version 1.0.0
 */

/* ─── System Includes ───────────────────────────────────────────────────── */
#include <stdio.h>
#include <time.h>

/* ─── Project Includes ──────────────────────────────────────────────────── */
#include <string.h>
#include "logger.h"

/* ─── Private State ─────────────────────────────────────────────────────── */

/* [MISRA VIOLATION] Rule 8.9 — variable with static storage duration
 * defined at file scope when it could be block-scoped */
static log_level_t g_log_level = LOG_DEBUG; /* MISRA Rule 8.9 */

/* [MISRA VIOLATION] Rule 7.4 — string literal assigned to non-const pointer */
static char *level_strings[] = {   /* MISRA Rule 7.4: should be const char * */
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR"
};

/* ─── Public: logger_set_level ─────────────────────────────────────────── */

void logger_set_level(log_level_t level)
{
    g_log_level = level;
}

/* ─── Public: logger_write ──────────────────────────────────────────────── */

void logger_write(log_level_t level,
                  const char *file,
                  int         line,
                  const char *msg)
{
    time_t     now;
    struct tm *tm_info;
    char       time_buf[32];

    if (level < g_log_level)
    {
        return;
    }

    if (msg == NULL)
    {
        return;
    }

    /* [MISRA VIOLATION] Rule 21.10 — use of time.h (date/time functions)
     * not recommended in safety-critical embedded code */
    now     = time(NULL);              /* MISRA Rule 21.10 */
    tm_info = localtime(&now);         /* MISRA Rule 21.10 */

    if (tm_info != NULL)
    {
        /* [MISRA VIOLATION] Rule 21.6 — strftime is stdio-adjacent */
        (void)strftime(time_buf, sizeof(time_buf),
                       "%H:%M:%S", tm_info); /* MISRA Rule 21.6 */
    }
    else
    {
        (void)strcpy(time_buf, "??:??:??");
    }

    /* [MISRA VIOLATION] Rule 21.6 — printf/fprintf in embedded code
     * Rule 17.7 — return value of printf not checked */
    printf("[%s][%s] %s:%d — %s\n",   /* MISRA Rule 21.6, Rule 17.7 */
           time_buf,
           level_strings[(int)level], /* MISRA Rule 10.5 — enum cast to int */
           file,
           line,
           msg);
}
