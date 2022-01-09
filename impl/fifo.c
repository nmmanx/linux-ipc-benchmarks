

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>

#include "ipc.h"

#define FIFO_PATH   "/tmp/fifo_ipc_benchmark"

struct fifo_context {
    const char *fifo_path;
    int rd_fd;
    int wr_fd;
    char *buffer;
};

static int fifo_setup_p(void **ctx, const testargs_t *args)
{
    *ctx = malloc(sizeof(struct fifo_context));
    struct fifo_context *pctx = (struct fifo_context*)*ctx;

    pctx->fifo_path = FIFO_PATH;
    pctx->buffer = (char*)calloc(args->blkSz, 1);

    int ret = mkfifo(pctx->fifo_path, S_IRUSR | S_IWUSR);
    if (ret < 0 && errno != EEXIST) {
        perror("mkfifo");
        return -1;
    }

    pctx->rd_fd = open(pctx->fifo_path, O_RDONLY | O_NONBLOCK);
    if (pctx->rd_fd < 0) {
        perror("open(rd_fd)");
        return -1;
    }

    return 0; 
}

static int fifo_revc_p(const void *ctx, const testargs_t *args, report_t *report)
{
    struct fifo_context *pctx = (struct fifo_context*)ctx;
    size_t nr = 0;
    
    while (nr < args->blkSz) {
        ssize_t ret = read(pctx->rd_fd, pctx->buffer, args->blkSz);
        if (ret < 0 && errno != EAGAIN) {
            perror("read");
            return -1;
        }
        if (ret > 0) {
            nr += ret;
        }
    }

    report->revc_sz += args->blkSz;
    return 0;
}

static int fifo_cleanup_p(const void *ctx)
{
    struct fifo_context *pctx = (struct fifo_context*)ctx;

    close(pctx->rd_fd);
    close(pctx->wr_fd);
    unlink(pctx->fifo_path);

    free(pctx->buffer);
    free(pctx);

    return 0;
}

static int fifo_setup_c(void **ctx, const testargs_t *args)
{
    struct fifo_context *pctx = (struct fifo_context*)*ctx;

    // Re-assign fifo path since FIFO_PATH is loaded to another location
    pctx->fifo_path = FIFO_PATH; 

    pctx->wr_fd = open(pctx->fifo_path, O_WRONLY);
    if (pctx->wr_fd < 0) {
        perror("open(wr_fd)");
        return -1;
    }

    memset(pctx->buffer, args->pattern, args->blkSz);
    close(pctx->rd_fd);

    return 0;
}

static int fifo_send_c(const void *ctx, const testargs_t *args)
{
    struct fifo_context *pctx = (struct fifo_context*)ctx;
    size_t nw = 0;

    while (nw < args->blkSz) {
        ssize_t ret = write(pctx->wr_fd, pctx->buffer, args->blkSz);
        if (ret < 0) {
            perror("write");
            return -1;
        }
        nw += ret;
    }

    return 0;
}

static int fifo_cleanup_c(const void *ctx)
{
    struct fifo_context *pctx = (struct fifo_context*)ctx;

    close(pctx->wr_fd);
    free(pctx->buffer);
    free(pctx);
    
    return 0;
}

static struct ipc_ops fifo_ipc_ops_p = {
    .setup = fifo_setup_p,
    .start = NULL,
    .send = NULL,
    .revc = fifo_revc_p,
    .clean = fifo_cleanup_p,
};

static struct ipc_ops fifo_ipc_ops_c = {
    .setup = fifo_setup_c,
    .start = NULL,
    .send = fifo_send_c,
    .revc = NULL,
    .clean = fifo_cleanup_c,
};

NEW_IPC_BENCHMARK(fifo_ipc, &fifo_ipc_ops_p, &fifo_ipc_ops_c)