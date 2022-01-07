#include <stdio.h>

#include "ipc.h"

int print_name(ipc_t *ipc)
{
    printf("Found IPC facility: %s\n", ipc->name);
    return 0;
}

int main(int argc, const char *argv[])
{
    ipc_for_each(print_name);

    ipc_t *ipc;
    ipc_get(0, &ipc);

    report_t rp;
    testargs_t args;

    args.blkSz = 4096;
    args.numBlks = 1000;

    ipc_execute(ipc, &args, &rp);

    printf("Execution time: %fms\n", rp.elapsed);
    printf("Bandwidth: %fMB/s\n", ((double)rp.revc_sz/1024/1024)/(rp.elapsed/1000.0));

    return 0;
}