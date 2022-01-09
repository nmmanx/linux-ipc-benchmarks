#include <sys/socket.h>
#include <sys/un.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "ipc.h"

#define SOCK_ABSTRACT_PATH  "tcp_un_ipc_benchmark"

struct tcp_un_context {
    int server_fd;
    int client_fd;
    const char *abs_path; // Abstract path
    char *buffer;
} g_tcp_un_context;

static int tcp_unix_setup_p(void **ctx, const testargs_t *args)
{
    g_tcp_un_context.abs_path = SOCK_ABSTRACT_PATH;
    g_tcp_un_context.buffer = (char*)malloc(args->blkSz);

    struct sockaddr_un addr;
    int ret;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    
    strncpy(addr.sun_path + 1, g_tcp_un_context.abs_path, sizeof(addr.sun_path) - 2);

    ret = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ret < 0) {
        perror("socket");
        return -1;
    }

    g_tcp_un_context.server_fd = ret;

    ret = bind(g_tcp_un_context.server_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if (ret < 0) {
        perror("bind");
        return -1;
    }

    ret = listen(g_tcp_un_context.server_fd, 1);
    if (ret < 0) {
        perror("listen");
        return -1;
    }

    *ctx = &g_tcp_un_context;
    return 0; 
}

static int tcp_unix_start_p(const void *ctx, const testargs_t *args)
{
    struct tcp_un_context *pctx = (struct tcp_un_context*)ctx;
    pctx->abs_path = SOCK_ABSTRACT_PATH;

    pctx->client_fd = accept(pctx->server_fd, NULL, 0); // Block until client connected
    if (pctx->client_fd < 0) {
        perror("accept");
        return -1;
    }

    return 0;
}

static int tcp_unix_revc_p(const void *ctx, const testargs_t *args, report_t *report)
{
    struct tcp_un_context *pctx = (struct tcp_un_context*)ctx;
    size_t nr = 0;
    
    while (nr < args->blkSz) {
        ssize_t ret = read(pctx->client_fd, pctx->buffer, args->blkSz);
        if (ret < 0) {
            perror("read");
            return -1;
        }
        nr += ret;
    }

    report->revc_sz += args->blkSz;
    return 0;
}

static int tcp_unix_cleanup_p(const void *ctx)
{
    struct tcp_un_context *pctx = (struct tcp_un_context*)ctx;

    close(pctx->client_fd);
    close(pctx->server_fd);
    free(pctx->buffer);

    return 0;
}

static int tcp_unix_setup_c(void **ctx, const testargs_t *args)
{
    struct tcp_un_context *pctx = (struct tcp_un_context*)*ctx;
    struct sockaddr_un addr;
    int ret;
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path + 1, pctx->abs_path, sizeof(addr.sun_path) - 2);

    close(pctx->server_fd);

    pctx->client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (pctx->client_fd < 0) {
        return -1;
    }

    ret = connect(pctx->client_fd, &addr, sizeof(struct sockaddr_un));
    if (ret < 0) {
        return -1;
    }

    memset(pctx->buffer, args->pattern, args->blkSz);
    return 0;
}

static int tcp_unix_send_c(const void *ctx, const testargs_t *args)
{
    struct tcp_un_context *pctx = (struct tcp_un_context*)ctx;
    size_t nw = 0;

    while (nw < args->blkSz) {
        ssize_t ret = write(pctx->client_fd, pctx->buffer, args->blkSz);
        if (ret < 0) {
            perror("write");
            return -1;
        }
        nw += ret;
    }

    return 0;
}

static int tcp_unix_cleanup_c(const void *ctx)
{
    struct tcp_un_context *pctx = (struct tcp_un_context*)ctx;

    close(pctx->client_fd);
    free(pctx->buffer);

    return 0;
}

static struct ipc_ops tcp_unix_ipc_ops_p = {
    .setup = tcp_unix_setup_p,
    .start = tcp_unix_start_p,
    .send = NULL,
    .revc = tcp_unix_revc_p,
    .clean = tcp_unix_cleanup_p,
};

static struct ipc_ops tcp_unix_ipc_ops_c = {
    .setup = tcp_unix_setup_c,
    .start = NULL,
    .send = tcp_unix_send_c,
    .revc = NULL,
    .clean = tcp_unix_cleanup_c,
};

NEW_IPC_BENCHMARK(tcp_unix_ipc, &tcp_unix_ipc_ops_p, &tcp_unix_ipc_ops_c)