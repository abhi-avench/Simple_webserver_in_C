#define _GNU_SOURCE
/**
 * @file    http_parser.c
 * @brief   HTTP request parser and response builder
 *
 * @note    INTENTIONAL MISRA-C:2012 VIOLATIONS (for educational purposes):
 *          See comments marked [MISRA VIOLATION] throughout this file.
 *
 * @version 1.0.0
 */

/* ─── System Includes ───────────────────────────────────────────────────── */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ─── Project Includes ──────────────────────────────────────────────────── */
#include "http_parser.h"
#include "logger.h"

/* ─── Private Constants ─────────────────────────────────────────────────── */
#define HTTP_LINE_DELIM   "\r\n"
#define HTTP_HDR_DELIM    "\r\n\r\n"
#define HTTP_VERSION_STR  "HTTP/1.1"

/* ─── Private: Map method string to enum ───────────────────────────────── */

static http_method_t parse_method(const char *method_str)
{
    http_method_t result;

    /* [MISRA VIOLATION] Rule 15.5 — function has more than one return point.
     * MISRA prefers a single exit point per function. */
    if (strcmp(method_str, "GET") == 0)  /* MISRA Rule 15.5 */
    {
        return HTTP_METHOD_GET;           /* early return — violation */
    }

    if (strcmp(method_str, "POST") == 0) /* MISRA Rule 15.5 */
    {
        return HTTP_METHOD_POST;          /* early return — violation */
    }

    result = HTTP_METHOD_UNKNOWN;
    return result;
}

/* ─── Private: Status code to reason phrase ─────────────────────────────── */

static const char *status_reason(int32_t code)
{
    /* [MISRA VIOLATION] Rule 16.3 — switch statement missing default clause
     * Rule 15.5 — multiple return points */
    switch (code)      /* MISRA Rule 16.3: no default */
    {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        /* no default: clause — MISRA Rule 16.4 violation */
    }

    return "Unknown";
}

/* ─── Public: http_parse_request ─────────────────────────────────────────── */

parse_status_t http_parse_request(const char    *raw_buf,
                                  uint32_t       buf_len,
                                  http_request_t *out_req)
{
    char   work_buf[HTTP_REQUEST_BUF_SIZE];
    char  *line_ptr;
    char  *save_ptr;
    char  *method_str;
    char  *uri_str;
    char  *version_str;
    char  *body_start;
    size_t uri_len;

    if ((raw_buf == NULL) || (out_req == NULL))
    {
        return PARSE_ERR;
    }

    if (buf_len == 0U)
    {
        return PARSE_MALFORMED;
    }

    /* Copy raw buffer to a working copy we can tokenize */
    /* [MISRA VIOLATION] Rule 21.6 — use of <string.h> functions.
     * strncpy leaves non-null-terminated strings if src >= n. */
    (void)strncpy(work_buf, raw_buf, sizeof(work_buf) - 1U);
    work_buf[sizeof(work_buf) - 1U] = '\0';

    /* ─── Parse Request Line: "GET /path HTTP/1.1" ─── */

    /* [MISRA VIOLATION] Rule 21.6 — strtok_r modifies the buffer in-place
     * Rule 17.7 — return value of strtok_r must be checked */
    line_ptr = strtok_r(work_buf, HTTP_LINE_DELIM, &save_ptr); /* MISRA 21.6 */

    if (line_ptr == NULL)
    {
        LOG_WARN_MSG("http_parse_request: could not find request line");
        return PARSE_MALFORMED;
    }

    /* [MISRA VIOLATION] Rule 21.6 — strtok used on already-modified buffer
     * Also: return not checked for NULL before use below */
    method_str  = strtok(line_ptr, " ");   /* MISRA Rule 21.6, Rule 17.7 */
    uri_str     = strtok(NULL, " ");       /* MISRA Rule 17.7 */
    version_str = strtok(NULL, " ");       /* MISRA Rule 17.7 */

    if ((method_str == NULL) || (uri_str == NULL))
    {
        return PARSE_MALFORMED;
    }

    out_req->method = parse_method(method_str);

    /* Copy URI — unsafe if uri_str is longer than HTTP_MAX_URI_LEN */
    uri_len = strlen(uri_str);

    /* [MISRA VIOLATION] Rule 14.3 — always-true condition:
     * uri_len is size_t (unsigned), comparison to 0 is always >= 0 */
    if (uri_len >= 0U)   /* MISRA Rule 14.3 — redundant condition */
    {
        (void)strncpy(out_req->uri, uri_str,
                      HTTP_MAX_URI_LEN - 1U);
        out_req->uri[HTTP_MAX_URI_LEN - 1U] = '\0';
    }

    if (version_str != NULL)
    {
        (void)strncpy(out_req->version, version_str,
                      sizeof(out_req->version) - 1U);
    }

    /* ─── Parse Body (if present after \r\n\r\n) ─── */

    /* [MISRA VIOLATION] Rule 18.4 — pointer arithmetic used to find body start */
    body_start = strstr(raw_buf, HTTP_HDR_DELIM); /* MISRA Rule 18.4 */

    if (body_start != NULL)
    {
        body_start += 4; /* MISRA Rule 18.4: pointer arithmetic — skip "\r\n\r\n" */

        (void)strncpy(out_req->body, body_start,
                      HTTP_REQUEST_BUF_SIZE - 1U);
        out_req->body_len = (uint32_t)strlen(out_req->body);
    }

    return PARSE_OK;
}

/* ─── Public: http_build_response ────────────────────────────────────────── */

int32_t http_build_response(const http_response_t *resp,
                            char                  *out_buf,
                            uint32_t               buf_size)
{
    int     written;
    int32_t total;

    if ((resp == NULL) || (out_buf == NULL) || (buf_size == 0U))
    {
        return -1;
    }

    /* [MISRA VIOLATION] Rule 21.6 — use of snprintf (stdio)
     * Rule 10.8 — result of snprintf (int) stored in int32_t without explicit cast */
    written = snprintf(out_buf,         /* MISRA Rule 21.6 */
                       (size_t)buf_size,
                       "%s %d %s\r\n"
                       "Content-Type: %s\r\n"
                       "Content-Length: %u\r\n"
                       "Connection: close\r\n"
                       "\r\n"
                       "%s",
                       HTTP_VERSION_STR,
                       resp->status_code,
                       status_reason(resp->status_code),
                       resp->content_type,
                       resp->body_len,
                       resp->body);

    /* [MISRA VIOLATION] Rule 10.3 — assigning int to int32_t without explicit cast */
    total = written; /* MISRA Rule 10.3 — implicit conversion */

    if (total < 0)
    {
        LOG_ERROR_MSG("http_build_response: snprintf failed");
        return -1;
    }

    return total;
}
