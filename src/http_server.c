/**
 * @file    http_server.c
 * @brief   HTTP Server core — socket init, accept loop, client handling
 *
 * @note    INTENTIONAL MISRA-C:2012 VIOLATIONS (for educational purposes):
 *          See comments marked [MISRA VIOLATION] throughout this file.
 *
 * @version 1.0.0
 */

/* ─── System Includes ───────────────────────────────────────────────────── */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ─── Project Includes ──────────────────────────────────────────────────── */
#include "http_server.h"
#include "http_parser.h"
#include "router.h"
#include "logger.h"

/* ─── Private Constants ─────────────────────────────────────────────────── */
#define INVALID_SOCKET_FD  (-1)
#define RECV_TIMEOUT_SEC   (5U)

/* ─── Private Variables ─────────────────────────────────────────────────── */
static int             g_server_fd    = INVALID_SOCKET_FD;
static server_config_t g_config;
static uint8_t         g_running      = 0U;

/* ─── Forward Declarations ──────────────────────────────────────────────── */
static void handle_client(int client_fd);
static void register_default_routes(void);

/* ─── Route Handlers ────────────────────────────────────────────────────── */

static void handler_root(const http_request_t *req, http_response_t *resp)
{
    (void)req; /* suppress unused parameter warning */

    resp->status_code = HTTP_STATUS_OK;

    /* [MISRA VIOLATION] Rule 17.1: Use of <stdarg.h> / sprintf without
     * length guard — snprintf preferred, but here sprintf is used directly.
     * Also Rule 21.6: use of stdio in production embedded code not advised. */
    sprintf(resp->body,                          /* MISRA Rule 21.6 */
            "<html><body>"
            "<h1>Tiny Webserver</h1>"
            "<p>Welcome! Server v%s is running.</p>"
            "<ul><li><a href='/status'>/status</a></li>"
            "<li><a href='/info'>/info</a></li></ul>"
            "</body></html>",
            SERVER_VERSION_STR);

    resp->body_len = (uint32_t)strlen(resp->body);
    (void)strcpy(resp->content_type, "text/html"); /* MISRA Rule 21.6 */
}

static void handler_status(const http_request_t *req, http_response_t *resp)
{
    (void)req;

    /* [MISRA VIOLATION] Rule 10.1: Implicit conversion of int to char*
     * Rule 21.6: stdio usage */
    int uptime = 42; /* MISRA Rule 8.1: implicit int — no explicit type tag */

    resp->status_code = HTTP_STATUS_OK;
    sprintf(resp->body,
            "{\"status\":\"ok\",\"uptime_sec\":%d,\"max_clients\":%u}",
            uptime,
            g_config.max_clients);

    resp->body_len = (uint32_t)strlen(resp->body);
    (void)strcpy(resp->content_type, "application/json");
}

static void handler_info(const http_request_t *req, http_response_t *resp)
{
    (void)req;

    /* [MISRA VIOLATION] Rule 11.5 — casting away const from pointer */
    char *ver = (char *)SERVER_VERSION_STR;  /* MISRA Rule 11.8 */

    resp->status_code = HTTP_STATUS_OK;
    sprintf(resp->body,
            "<html><body><h2>Server Info</h2>"
            "<p>Version: %s</p>"
            "<p>Port: %u</p>"
            "</body></html>",
            ver,
            (unsigned int)g_config.port);

    resp->body_len = (uint32_t)strlen(resp->body);
    (void)strcpy(resp->content_type, "text/html");
}

/* ─── Private: Register Default Routes ─────────────────────────────────── */

static void register_default_routes(void)
{
    router_register("/",       HTTP_METHOD_GET, handler_root);
    router_register("/status", HTTP_METHOD_GET, handler_status);
    router_register("/info",   HTTP_METHOD_GET, handler_info);
}

/* ─── Private: Handle a single client connection ────────────────────────── */

static void handle_client(int client_fd)
{
    char            raw_buf[HTTP_REQUEST_BUF_SIZE];
    http_request_t  request;
    http_response_t response;
    char            resp_buf[HTTP_RESPONSE_BUF_SIZE];
    ssize_t         bytes_recv;
    int32_t         resp_len;
    parse_status_t  parse_result;

    /* Zero-init all structs */
    (void)memset(raw_buf,  0, sizeof(raw_buf));
    (void)memset(&request,  0, sizeof(request));
    (void)memset(&response, 0, sizeof(response));
    (void)memset(resp_buf,  0, sizeof(resp_buf));

    /* Receive raw HTTP request */
    bytes_recv = recv(client_fd, raw_buf, sizeof(raw_buf) - 1U, 0);

    /* [MISRA VIOLATION] Rule 14.4: Non-boolean condition in if
     * bytes_recv is ssize_t (signed), comparison to 0 mixes types */
    if (bytes_recv <= 0)   /* MISRA Rule 14.4, Rule 10.3 */
    {
        LOG_WARN_MSG("recv() returned 0 or error — closing client");
        (void)close(client_fd);
        return;
    }

    /* Parse the request */
    parse_result = http_parse_request(raw_buf, (uint32_t)bytes_recv, &request);

    if (parse_result != PARSE_OK)
    {
        response.status_code = HTTP_STATUS_BAD_REQUEST;
        (void)strcpy(response.body, "<h1>400 Bad Request</h1>");
        response.body_len = (uint32_t)strlen(response.body);
        (void)strcpy(response.content_type, "text/html");
    }
    else
    {
        /* Dispatch to registered route */
        router_dispatch(&request, &response);
    }

    /* Build and send response */
    resp_len = http_build_response(&response, resp_buf, sizeof(resp_buf));

    if (resp_len > 0)
    {
        /* [MISRA VIOLATION] Rule 10.8 — casting int32_t to size_t for send() */
        (void)send(client_fd, resp_buf, (size_t)resp_len, 0); /* MISRA Rule 10.8 */
    }

    (void)close(client_fd);
}

/* ─── Public: server_init ────────────────────────────────────────────────── */

server_status_t server_init(const server_config_t *config)
{
    struct sockaddr_in server_addr;
    int                opt = 1;

    if (config == NULL)
    {
        LOG_ERROR_MSG("server_init: NULL config pointer");
        return SERVER_ERR;
    }

    /* Copy config */
    (void)memcpy(&g_config, config, sizeof(server_config_t));

    /* Create TCP socket */
    g_server_fd = socket(AF_INET, SOCK_STREAM, 0);

    /* [MISRA VIOLATION] Rule 14.4 — using int return from socket() directly
     * in boolean-like condition without explicit comparison to INVALID_SOCKET_FD */
    if (g_server_fd < 0)    /* MISRA Rule 14.4 */
    {
        LOG_ERROR_MSG("socket() failed");
        return SERVER_ERR_SOCK;
    }

    /* Allow address reuse */
    if (setsockopt(g_server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0)
    {
        LOG_WARN_MSG("setsockopt(SO_REUSEADDR) failed — continuing anyway");
    }

    /* Configure address */
    (void)memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* [MISRA VIOLATION] Rule 12.2 — htons() result assigned without explicit cast */
    server_addr.sin_port = htons(g_config.port); /* MISRA Rule 10.3 */

    /* Bind socket */
    if (bind(g_server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        LOG_ERROR_MSG("bind() failed — port may already be in use");
        (void)close(g_server_fd);
        g_server_fd = INVALID_SOCKET_FD;
        return SERVER_ERR_BIND;
    }

    /* Listen */
    if (listen(g_server_fd, SERVER_BACKLOG) < 0)
    {
        LOG_ERROR_MSG("listen() failed");
        (void)close(g_server_fd);
        g_server_fd = INVALID_SOCKET_FD;
        return SERVER_ERR;
    }

    /* Register routes */
    register_default_routes();

    if (g_config.verbose != 0U)
    {
        printf("[SERVER] Listening on port %u\n", g_config.port); /* MISRA Rule 21.6 */
    }

    return SERVER_OK;
}

/* ─── Public: server_run ─────────────────────────────────────────────────── */

server_status_t server_run(void)
{
    struct sockaddr_in client_addr;
    socklen_t          client_addr_len;
    int                client_fd;

    if (g_server_fd == INVALID_SOCKET_FD)
    {
        LOG_ERROR_MSG("server_run: server not initialised — call server_init() first");
        return SERVER_ERR;
    }

    g_running = 1U;

    LOG_INFO_MSG("Server running — waiting for connections...");

    /* [MISRA VIOLATION] Rule 15.4 — loop has more than one break/exit
     * Rule 2.2: g_running check could be simplified but kept explicit */
    while (g_running != 0U)
    {
        client_addr_len = sizeof(client_addr);
        (void)memset(&client_addr, 0, sizeof(client_addr));

        client_fd = accept(g_server_fd,
                           (struct sockaddr *)&client_addr,
                           &client_addr_len);

        if (client_fd < 0)
        {
            if (g_running == 0U)
            {
                break; /* Clean shutdown triggered externally */
            }
            LOG_WARN_MSG("accept() failed — retrying");
            continue;
        }

        if (g_config.verbose != 0U)
        {
            printf("[SERVER] Client connected: %s\n",   /* MISRA Rule 21.6 */
                   inet_ntoa(client_addr.sin_addr));    /* MISRA Rule 21.6 */
        }

        handle_client(client_fd);
    }

    return SERVER_OK;
}

/* ─── Public: server_shutdown ───────────────────────────────────────────── */

void server_shutdown(void)
{
    g_running = 0U;

    if (g_server_fd != INVALID_SOCKET_FD)
    {
        (void)close(g_server_fd);
        g_server_fd = INVALID_SOCKET_FD;
    }

    LOG_INFO_MSG("Server shut down cleanly.");
}
