#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "ipc.h"

int execute(const ipc_t* ipc, const testargs_t* args, report_t *report)
{
    void *ctx;
    int ret;

    report->stat = -1;

    ret = ipc->ops->setup_p(&ctx, args);
    if (ret) {
        return ret;
    }

    pid_t pid = fork();
    
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        size_t n = 0;
        if (ipc->ops->setup_c(ctx, args) < 0) {
            _exit(1);
        }
        while (n < args->nblks) {
            if (ipc->ops->send_c(ctx, args) < 0) {
                _exit(1);
            }
            n++;
        }
        ipc->ops->cleanup_c(ctx);
        _exit(0);
    } else {
        struct timeval t1;
        struct timeval t2;
        int stat;
        size_t n = 0;
        report->revc_sz = 0;

        gettimeofday(&t1, NULL);
        while (n < args->nblks) {
            if (ipc->ops->revc_p(ctx, args, report) < 0) {
                return -1;
            }
            n++;
        }
        gettimeofday(&t2, NULL);

        wait(NULL); // TODO: check child status
        ipc->ops->cleanup_p(ctx);

        if (report->revc_sz != (args->blksz * args->nblks)) {
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