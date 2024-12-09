
#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/spinlock.h"
#include "kernel/riscv.h"
#include "kernel/proc.h"
#include "user/user.h"

int main(void){
    struct pstat st;

    if (getpinfo(&st) != 0) {
        printf("ps: cannot get info\n");
        exit(1);
    }

    printf("PID\tPriority\n");

    for (int i = 0; i < NPROC; i++){
        if (st.inuse[i]){
            printf("%d\t%d\n",st.pid[i],st.priority[i]);
        }
    }

    exit(0);
}

