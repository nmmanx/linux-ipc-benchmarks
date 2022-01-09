#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "ipc.h"

#define IPC_ENTRY_ALIGNMENT     32

extern void *__ipc_array_start;
extern void *__ipc_array_end;

static int ipc_get_arr_offs()
{
    int offs;

    if (sizeof(ipc_t) % IPC_ENTRY_ALIGNMENT != 0) {
        offs = (sizeof(ipc_t) + IPC_ENTRY_ALIGNMENT) / IPC_ENTRY_ALIGNMENT;
        offs *= IPC_ENTRY_ALIGNMENT;
    } else {
        offs = sizeof(ipc_t);
    }

    return offs / sizeof(void*);
}

int ipc_execute(const ipc_t* ipc, const testargs_t* args, report_t *report)
{
    void *ctx;
    int ret;

    report->stat = -1;
    report->ipc = ipc;

    ret = ipc->parent_ops->setup(&ctx, args);
    if (ret) {
        return ret;
    }

    pid_t pid = fork();
    
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        // TODO: child process should signal parent when error occured
        size_t n = 0;
        if (ipc->child_ops->setup(&ctx, args) < 0) {
            _exit(1);
        }
        if (ipc->child_ops->start && ipc->child_ops->start(&ctx, args) < 0) {
            _exit(1);
        }
        while (n < args->numBlks) {
            if (ipc->child_ops->send(ctx, args) < 0) {
                _exit(1);
            }
            n++;
        }
        ipc->child_ops->clean(ctx);
        _exit(0);
    } else {
        struct timeval t1;
        struct timeval t2;
        int stat;
        size_t n = 0;
        report->revc_sz = 0;

        if (ipc->parent_ops->start && ipc->parent_ops->start(ctx, args) < 0) {
            return -1;
        }

        gettimeofday(&t1, NULL);
        while (n < args->numBlks) {
            if (ipc->parent_ops->revc(ctx, args, report) < 0) {
                return -1;
            }
            n++;
        }
        gettimeofday(&t2, NULL);

        wait(NULL); // TODO: check child status
        ipc->parent_ops->clean(ctx);

        if (report->revc_sz != (args->blkSz * args->numBlks)) {
            report->stat = -1;
            printf("Failure tranmission\n");
            return -1;
        } else {
            report->stat = 0;
            report->elapsed = (t2.tv_sec - t1.tv_sec) * 1000.0;
            report->elapsed += (t2.tv_usec - t1.tv_usec) / 1000.0;
        }
    }

    return 0;
}

int ipc_count()
{
    uintptr_t start = (uintptr_t)&__ipc_array_start;
    uintptr_t end = (uintptr_t)&__ipc_array_end; 

    if (end % IPC_ENTRY_ALIGNMENT == 0) {
        return (end - start) / IPC_ENTRY_ALIGNMENT;
    } else {
        return (end - start) / IPC_ENTRY_ALIGNMENT + 1;
    }
}

int ipc_get(int id, ipc_t **ipc)
{
    int sz = ipc_count();
    int offs = ipc_get_arr_offs();

    if (id >= 0 && id < sz) {
        *ipc = (ipc_t*)(&__ipc_array_start + id * offs);
        return 0;
    }

    return -1;
}

void ipc_for_each(int(*cb)(ipc_t*, int))
{
    int n = ipc_get_arr_offs();
    int i = 0;
    void *p = &__ipc_array_start;

    do {
        p = &__ipc_array_start + (i++)*n;
    } while ((uintptr_t)p < (uintptr_t)&__ipc_array_end && !cb(p, i - 1));
}