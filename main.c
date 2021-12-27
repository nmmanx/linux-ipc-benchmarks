#include <stdio.h>

#include "ipc.h"

extern void *__ipc_array_start;
extern void *__ipc_array_end;

int main()
{
    int i = 0;
    while (&__ipc_array_start + i*sizeof(void*) < &__ipc_array_end) {
        ipc_t *ipcarr = (ipc_t*)(&__ipc_array_start + i*4);
        printf("name: %s\n", ipcarr->name);
        i++;
    }
    return 0;
}