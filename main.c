#include <stdio.h>

#include <stdio.h>
#include "ipc.h"

extern void *__ipc_array_start;
extern void *__ipc_array_end;

int main()
{
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