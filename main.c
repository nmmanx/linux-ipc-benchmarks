#include <stdio.h>

#include <stdio.h>
#include "ipc.h"

#define IPC_ENTRY_ALIGNMENT   32

extern void *__ipc_array_start;
extern void *__ipc_array_end;

unsigned int get_num_ipc()
{
    return (&__ipc_array_end - &__ipc_array_start) / IPC_ENTRY_ALIGNMENT + 1;
}

void for_each_ipc(int(*cb)(ipc_t*))
{
    static const int align = IPC_ENTRY_ALIGNMENT;
    int n = ((sizeof(ipc_t) + align) - (sizeof(ipc_t) % align)) / sizeof(void*);
    int i = 0;
    void *p = &__ipc_array_start;

    do {
        p = &__ipc_array_start + (i++)*n;
    } while (p < &__ipc_array_end && !cb(p));
}

int print_name(ipc_t *ipc)
{
    printf("Found IPC facility: %s\n", ipc->name);
    return 0;
}

int main()
{
    for_each_ipc(print_name);

    ipc_t *ipc = (ipc_t*)&__ipc_array_start;

    report_t rp;
    testargs_t args;

    args.blkSz = 4096;
    args.numBlks = 1000;

    execute(ipc, &args, &rp);

    printf("Execution time: %fms\n", rp.elapsed);
    printf("Bandwidth: %fMB/s\n", ((double)rp.revc_sz/1024/1024)/(rp.elapsed/1000.0));

    return 0;
}