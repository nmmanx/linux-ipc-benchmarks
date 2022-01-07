#ifndef __IPC_H__
#define __IPC_H__

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Macro to declare a new IPC facility
 * 
 * This macro should be called in the IPC's c file to add
 * a new ipc_t to the .ipc_array section.
 */
#define NEW_IPC_BENCHMARK(NAME, PARENT_OPS, CHILD_OPS) \
    ipc_t NAME __attribute__((section(".ipc_array"))) = { \
        .enabled = true, \
        .name = #NAME, \
        .parent_ops = PARENT_OPS, \
        .child_ops = CHILD_OPS \
    };

/**
 * @brief Arguments for testing bandwidth
 * 
 */
typedef struct {
    char pattern;
    size_t numBlks;
    size_t blkSz;
} testargs_t;

/**
 * @brief Report data structure
 * 
 */
typedef struct {
    int stat;
    size_t revc_sz;
    double elapsed;
} report_t;

struct ipc_ops {
    int (*setup)(void **ctx, const testargs_t*);
    int (*send)(const void *ctx, const testargs_t*);
    int (*revc)(const void *ctx, const testargs_t*, report_t *report);
    int (*clean)(const void *ctx);
};

/**
 * @brief IPC facility data structure
 * 
 */
typedef struct {
    bool enabled;
    const char *name;
    struct ipc_ops *child_ops;
    struct ipc_ops *parent_ops;
} ipc_t;

/**
 * @brief Execute benchmark of an IPC facility
 * 
 * @param ipc The IPC facility to perform testing
 * @param args Test arguments
 * @param report Output report
 * @return int 
 */
int execute(const ipc_t* ipc, const testargs_t* args, report_t *report);

#endif // __IPC_H__