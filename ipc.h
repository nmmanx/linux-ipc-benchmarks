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
        .enabled = false, \
        .name = #NAME, \
        .parent_ops = PARENT_OPS, \
        .child_ops = CHILD_OPS, \
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
    ipc_t *ipc;
    int stat;
    size_t revc_sz;
    double elapsed;
} report_t;

/**
 * @brief IPC facility operation
 * 
 * Both parent and child process has their own operations
 */
struct ipc_ops {
    int (*setup)(void **ctx, const testargs_t*);
    int (*send)(const void *ctx, const testargs_t*);
    int (*revc)(const void *ctx, const testargs_t*, report_t *report);
    int (*clean)(const void *ctx);
};

/**
 * @brief Execute benchmark of an IPC facility
 * 
 * @param ipc The IPC facility to perform testing
 * @param args Test arguments
 * @param report Output report
 * @return int 
 */
int ipc_execute(const ipc_t* ipc, const testargs_t* args, report_t *report);

/**
 * @brief Returns the number of implemented IPC facilities
 * 
 * @return Number of implemented IPC
 */
int ipc_count();

/**
 * @brief Get IPC instance
 * 
 * @param id ID of IPC in the array
 * @param ipc Output pointer
 * @return Returns 0 on sucess, -1 on failure
 */
int ipc_get(int id, ipc_t **ipc);

/**
 * @brief Traversal all IPC facilities
 * 
 * @param cb Handler to process each IPC instance
 */
void ipc_for_each(int(*cb)(ipc_t*, int));

#endif // __IPC_H__