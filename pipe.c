#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "ipc.h"

struct pipe_ctx {
    int fd[2];
    char *data;
};

static int pipe_setup_p(void **ctx, const testargs_t *args)
{
    *ctx = (struct pipe_ctx*)malloc(sizeof(struct pipe_ctx));

    int ret = pipe(((struct pipe_ctx*)*ctx)->fd);
    if (ret) {
        perror("pipe");
        return ret;
    }

    ((struct pipe_ctx*)*ctx)->data = (char*)calloc(args->blksz, sizeof(char));
    return 0; 
}

static int pipe_setup_c(const void *ctx, const testargs_t *args)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)ctx;
    pctx->data = (char*)calloc(args->blksz, sizeof(char));
    memset(pctx->data, args->pattern, args->blksz);
    return 0;
}

static int pipe_revc_p(const void *ctx, const testargs_t *args, report_t *report)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)ctx;
    size_t nr = 0;
    
    while (nr < args->blksz) {
        ssize_t ret = read(pctx->fd[0], pctx->data, args->blksz);
        if (ret < 0) {
            perror("read");
            return -1;
        }
        nr += ret;
    }

    report->revc_sz += args->blksz;
    return 0;
}

static int pipe_send_c(const void *ctx, const testargs_t *args)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)ctx;
    size_t nw = 0;

    while (nw < args->blksz) {
        ssize_t ret = write(pctx->fd[1], pctx->data, args->blksz);
        if (ret < 0) {
            perror("write");
            return -1;
        }
        nw += ret;
    }

    return 0;
}

static int pipe_cleanup_p(const void *ctx)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)ctx;

    free(pctx->data);
    close(pctx->fd[0]);

    return 0;
}

static int pipe_cleanup_c(const void *ctx)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)ctx;

    free(pctx->data);
    close(pctx->fd[1]);
    
    return 0;
}

static struct ipc_ops pipe_ipc_ops = {
    .setup_p = pipe_setup_p,
    .setup_c = pipe_setup_c,
    .revc_p = pipe_revc_p,
    .send_c = pipe_send_c,
    .cleanup_p = pipe_cleanup_p,
    .cleanup_c = pipe_cleanup_c
};

IPC_BENCHMARK(pipe_ipc, &pipe_ipc_ops)