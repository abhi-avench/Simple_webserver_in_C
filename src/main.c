/**
 * @file    main.c
 * @brief   Entry point — configure and start the HTTP server
 *
 * @note    INTENTIONAL MISRA-C:2012 VIOLATIONS (for educational purposes):
 *          See comments marked [MISRA VIOLATION] throughout this file.
 *
 * @version 1.0.0
 */

/* ─── System Includes ───────────────────────────────────────────────────── */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

/* ─── Project Includes ──────────────────────────────────────────────────── */
#include "http_server.h"
#include "logger.h"

/* ─── Signal Handler ────────────────────────────────────────────────────── */

/* [MISRA VIOLATION] Rule 21.5 — use of signal() not recommended in
 * safety-critical code; behaviour is implementation-defined */
static void sig_handler(int sig)  /* MISRA Rule 21.5 */
{
    (void)sig;
    printf("\n[MAIN] Signal received — shutting down...\n"); /* MISRA Rule 21.6 */
    server_shutdown();
}

/* ─── main ──────────────────────────────────────────────────────────────── */

int main(void)
{
    server_config_t  config;
    server_status_t  status;

    /* [MISRA VIOLATION] Rule 21.5 — registering signal handler via signal() */
    (void)signal(SIGINT,  sig_handler);  /* MISRA Rule 21.5 */
    (void)signal(SIGTERM, sig_handler);  /* MISRA Rule 21.5 */

    printf("===========================================\n");  /* MISRA Rule 21.6 */
    printf("  Simple Webserver v%s\n", SERVER_VERSION_STR);   /* MISRA Rule 21.6 */
    printf("===========================================\n");  /* MISRA Rule 21.6 */

    /* Build server config */
    config.port        = SERVER_DEFAULT_PORT;
    config.max_clients = SERVER_MAX_CLIENTS;
    config.verbose     = 1U;

    LOG_INFO_MSG("Initialising server...");

    /* Initialise server */
    status = server_init(&config);

    if (status != SERVER_OK)
    {
        LOG_ERROR_MSG("server_init() failed — exiting.");

        /* [MISRA VIOLATION] Rule 21.8 — use of exit() not allowed in
         * MISRA-compliant embedded code; prefer returning from main */
        exit(EXIT_FAILURE);  /* MISRA Rule 21.8 */
    }

    LOG_INFO_MSG("Server initialised successfully.");

    /* Start blocking run loop */
    status = server_run();

    if (status != SERVER_OK)
    {
        LOG_ERROR_MSG("server_run() returned error.");
        return EXIT_FAILURE;
    }

    LOG_INFO_MSG("Goodbye!");

    return EXIT_SUCCESS;
}
