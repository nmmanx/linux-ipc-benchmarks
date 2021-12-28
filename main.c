#include <stdio.h>

#include <stdio.h>
#include "ipc.h"

extern void *__ipc_array_start;
extern void *__ipc_array_end;

void for_each_ipc(int(*cb)(ipc_t*))
{
    static const int align = 32;
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

    args.blksz = 4096;
    args.nblks = 1000;

    execute(ipc, &args, &rp);

    printf("Execution time: %fms\n", rp.elapsed);
    printf("Bandwidth: %fMB/s\n", ((double)rp.revc_sz/1024/1024)/(rp.elapsed/1000.0));

    return 0;
}