#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "ipc.h"

#define DEFAULT_BLOCK_SIZE  4096
#define DEFAULT_BLOCK_NUM   1024

static int print_ipc(ipc_t *ipc, int id)
{
    printf("[%d]\t%s\n", id, ipc->name);
    return 0;
}

void print_help()
{
    // TODO: add help messages
    static const char *msg = "" \
        "" \
        "";

    puts(msg);
}

void print_ipc_list()
{
    puts("IPC Facilities:");
    ipc_for_each(print_ipc);
}

void set_ipc_enabled(int id, bool enabled)
{
    ipc_t *ipc = NULL;
    if (ipc_get(id, &ipc) == 0) {
        ipc->enabled = enabled;
    } else {
        fprintf(stderr, "IPC id %d not found\n", id);
    }
}

int enabled_ipc(ipc_t *ipc, int id)
{
    ipc->enabled = true;
    return 0;
}

static int report_comparator(const report_t *rp1, const report_t *rp2)
{
    if (rp1->elapsed > rp2->elapsed) {
        return 1;
    } else if (rp1->elapsed < rp2->elapsed) {
        return -1;
    }
    return 0;
}

void print_reports(report_t *reports, int sz)
{
    qsort(reports, sz, sizeof(report_t), report_comparator);
    puts("Result:\n");

    for (int i = 0; i < sz; ++i) {
        double bw = reports[i].revc_sz / (1024 * 1024) / (reports[i].elapsed / 1000);
        printf("%d.\t%s\t%f (ms)\t%f (MB/s)\n", i + 1, reports[i].ipc->name, reports[i].elapsed, bw);
    }
}

int main(int argc, char *argv[])
{
    int opt;
    const int ipcCount = ipc_count();
    bool verbose = false; // TODO: verbose mode
    size_t blkSz =DEFAULT_BLOCK_SIZE;
    size_t numBlks = DEFAULT_BLOCK_NUM;
    int enabled_cout = 0;
    report_t *reports;
    char *tk = NULL;

    while ((opt = getopt(argc, argv, "vhls:b:n:")) != -1) {
        switch (opt)
        {
        case 'v': /* Verbose mode */
            verbose = true;
            break;
        case 'h': /* Print help message */
            print_help();
            _exit(EXIT_SUCCESS);
            break;
        case 'l': /* List all IPC facilities */
            print_ipc_list();
            _exit(EXIT_SUCCESS);
            break;
        case 's': /* Select which IPC to run, e.g. "1,2,3" or "1:2:3" */
            tk = strtok(optarg, ",:");
            while (tk != NULL) {
                char *p = NULL;
                int id = strtol(tk, &p, 10);
                if (p != NULL && *p == '\0') {
                    set_ipc_enabled(id, true);
                    enabled_cout++;
                } else {
                    fprintf(stderr, "Invalid IPC id: %s\n", tk);
                }
                tk = strtok(NULL, ",:");
            }
            break;
        case 'b': /* Specify block size */
            do {
                char *p = NULL;
                int n = strtol(optarg, &p, 10);
                if (p != NULL && *p == '\0') {
                    blkSz = n;
                } else {
                    fprintf(stderr, "Invalid block size: %s\n", optarg);
                    _exit(EXIT_FAILURE);
                }
            } while (0);
            break;
        case 'n': /* Specify number of blocks */
            do {
                char *p = NULL;
                int n = strtol(optarg, &p, 10);
                if (p != NULL && *p == '\0') {
                    numBlks = n;
                } else {
                    fprintf(stderr, "Invalid number of blocks: %s\n", optarg);
                    _exit(EXIT_FAILURE);
                }
            } while (0);
            break;
        default:
            break;
        }
    }

    if (enabled_cout < 1) {
        ipc_for_each(enabled_ipc);
        enabled_cout = ipcCount;
    }

    reports = (report_t*)calloc(enabled_cout, sizeof(report_t));
    testargs_t args;
    int i = 0;
    int ret;

    args.pattern =  0xAB; // TODO: allow providing pattern, verify pattern?
    args.blkSz = blkSz;
    args.numBlks = numBlks;

    for (int i = 0; i < ipcCount; ++i) {
        ipc_t *ipc = NULL;
        ret = ipc_get(i, &ipc);

        if (ipc != NULL && ipc->enabled) {
            printf("Execute: %s...\n", ipc->name);
            ret = ipc_execute(ipc, &args, reports + i);
            if (ret < 0) {
                fprintf(stderr, "Error occured while executing %s\n", ipc->name);
                _exit(EXIT_FAILURE);
            }
        }
    }

    print_reports(reports, enabled_cout);
    free(reports);

    return 0;
}