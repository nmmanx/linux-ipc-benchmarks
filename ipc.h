#ifndef __IPC_H__
#define __IPC_H__

#include <stddef.h>
#include <stdbool.h>

#define IPC_BENCHMARK(NAME, OPS) \
    ipc_t NAME __attribute__((section(".ipc_array"))) = { \
        .enabled = true, \
        .name = #NAME, \
        .ops = OPS , \
    };

typedef struct {
    char pattern;
    size_t nblks;
    size_t blksz;
} testargs_t;

typedef struct {
    int stat;
    size_t revc_sz;
    double elapsed;
} report_t;

struct ipc_ops {
    int (*setup_p)(void **ctx, const testargs_t*);
    int (*setup_c)(const void *ctx, const testargs_t*);
    int (*revc_p)(const void *ctx, const testargs_t*, report_t *report);
    int (*send_c)(const void *ctx, const testargs_t*);
    int (*cleanup_p)(const void *ctx);
    int (*cleanup_c)(const void *ctx);
};

typedef struct {
    bool enabled;
    const char *name;
    struct ipc_ops *ops;
} ipc_t;

int execute(const ipc_t* ipc, const testargs_t* args, report_t *report);

#endif // __IPC_H__