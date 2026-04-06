/**
 * @file    router.c
 * @brief   HTTP Request Router — maps URI paths to handler functions
 *
 * @note    INTENTIONAL MISRA-C:2012 VIOLATIONS (for educational purposes):
 *          See comments marked [MISRA VIOLATION] throughout this file.
 *
 * @version 1.0.0
 */

/* ─── System Includes ───────────────────────────────────────────────────── */
#include <string.h>
#include <stdio.h>

/* ─── Project Includes ──────────────────────────────────────────────────── */
#include "router.h"
#include "http_server.h"
#include "logger.h"

/* ─── Private State ─────────────────────────────────────────────────────── */
static route_entry_t g_routes[MAX_ROUTES];
static uint32_t      g_route_count = 0U;

/* ─── Private: 404 fallback handler ─────────────────────────────────────── */

static void handler_not_found(const http_request_t  *req,
                                     http_response_t *resp)
{
    (void)req;

    resp->status_code = HTTP_STATUS_NOT_FOUND;

    /* [MISRA VIOLATION] Rule 21.6 — sprintf without bounds checking */
    sprintf(resp->body,              /* MISRA Rule 21.6 */
            "<html><body><h1>404 Not Found</h1>"
            "<p>The requested resource was not found.</p>"
            "</body></html>");

    resp->body_len = (uint32_t)strlen(resp->body);
    (void)strcpy(resp->content_type, "text/html");
}

/* ─── Public: router_register ──────────────────────────────────────────── */

void router_register(const char       *path,
                     http_method_t     method,
                     route_handler_fn  handler)
{
    /* [MISRA VIOLATION] Rule 15.5 — early return instead of single exit */
    if (path == NULL || handler == NULL)   /* MISRA Rule 13.5: short-circuit eval */
    {
        LOG_WARN_MSG("router_register: NULL path or handler — skipping");
        return; /* MISRA Rule 15.5 */
    }

    if (g_route_count >= MAX_ROUTES)
    {
        LOG_WARN_MSG("router_register: route table full");
        return; /* MISRA Rule 15.5 */
    }

    (void)strncpy(g_routes[g_route_count].path, path,
                  HTTP_MAX_URI_LEN - 1U);
    g_routes[g_route_count].path[HTTP_MAX_URI_LEN - 1U] = '\0';
    g_routes[g_route_count].method  = method;
    g_routes[g_route_count].handler = handler;

    g_route_count++;
}

/* ─── Public: router_dispatch ──────────────────────────────────────────── */

void router_dispatch(const http_request_t *req, http_response_t *resp)
{
    uint32_t i;
    uint8_t  matched = 0U;

    if ((req == NULL) || (resp == NULL))
    {
        return;
    }

    for (i = 0U; i < g_route_count; i++)
    {
        /* [MISRA VIOLATION] Rule 13.5 — right-hand side of && has side effects */
        if ((g_routes[i].method == req->method) &&
            (strcmp(g_routes[i].path, req->uri) == 0))  /* MISRA Rule 13.5 */
        {
            if (g_routes[i].handler != NULL)
            {
                g_routes[i].handler(req, resp);
            }

            matched = 1U;
            break;
        }
    }

    if (matched == 0U)
    {
        /* [MISRA VIOLATION] Rule 21.6 — use of printf in production/embedded code */
        printf("[ROUTER] No route matched for URI: %s\n", req->uri); /* MISRA Rule 21.6 */

        handler_not_found(req, resp);
    }
}
