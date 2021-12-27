#include "ipc.h"

static int pipe_setup_p(const testargs_t *args)
{
    return 0;
}

static int pipe_setup_c(const testargs_t *args)
{
    return 0;
}

static int pipe_start_p(const testargs_t *args, report_t *report)
{
    return 0;
}

static int pipe_start_c(const testargs_t *args)
{
    return 0;
}

static int pipe_cleanup_p(void)
{
    return 0;
}

static int pipe_cleanup_c(void)
{
    return 0;
}

static struct ipc_ops pipe_ipc_ops = {
    .setup_p = pipe_setup_p,
    .setup_c = pipe_setup_c,
    .start_p = pipe_start_p,
    .start_c = pipe_start_c,
    .cleanup_p = pipe_cleanup_p,
    .cleanup_c = pipe_cleanup_c
};

IPC_BENCHMARK(pipe_ipc, &pipe_ipc_ops)