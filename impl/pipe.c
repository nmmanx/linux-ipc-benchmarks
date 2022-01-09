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
    *ctx = malloc(sizeof(struct pipe_ctx));
    struct pipe_ctx *pctx = (struct pipe_ctx *)*ctx;

    int ret = pipe(pctx->fd);
    if (ret) {
        perror("pipe");
        return ret;
    }

    pctx->data = (char*)calloc(args->blkSz, sizeof(char));

    return 0; 
}

static int pipe_revc_p(const void *ctx, const testargs_t *args, report_t *report)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)ctx;
    size_t nr = 0;
    
    while (nr < args->blkSz) {
        ssize_t ret = read(pctx->fd[0], pctx->data, args->blkSz);
        if (ret < 0) {
            perror("read");
            return -1;
        }
        nr += ret;
    }

    report->revc_sz += args->blkSz;
    return 0;
}

static int pipe_cleanup_p(const void *ctx)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)ctx;

    close(pctx->fd[0]);
    close(pctx->fd[1]);
    free(pctx->data);
    free(pctx);

    return 0;
}

static int pipe_setup_c(void **ctx, const testargs_t *args)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)*ctx;
    
    pctx->data = (char*)calloc(args->blkSz, sizeof(char));
    memset(pctx->data, args->pattern, args->blkSz);
    close(pctx->fd[0]); // close unused read end

    return 0;
}

static int pipe_send_c(const void *ctx, const testargs_t *args)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)ctx;
    size_t nw = 0;

    while (nw < args->blkSz) {
        ssize_t ret = write(pctx->fd[1], pctx->data, args->blkSz);
        if (ret < 0) {
            perror("write");
            return -1;
        }
        nw += ret;
    }

    return 0;
}

static int pipe_cleanup_c(const void *ctx)
{
    struct pipe_ctx *pctx = (struct pipe_ctx*)ctx;

    close(pctx->fd[1]);
    free(pctx->data);
    free(pctx);
    
    return 0;
}

static struct ipc_ops pipe_ipc_ops_p = {
    .setup = pipe_setup_p,
    .start = NULL,
    .send = NULL,
    .revc = pipe_revc_p,
    .clean = pipe_cleanup_p,
};

static struct ipc_ops pipe_ipc_ops_c = {
    .setup = pipe_setup_c,
    .start = NULL,
    .send = pipe_send_c,
    .revc = NULL,
    .clean = pipe_cleanup_c,
};

NEW_IPC_BENCHMARK(pipe_ipc, &pipe_ipc_ops_p, &pipe_ipc_ops_c)