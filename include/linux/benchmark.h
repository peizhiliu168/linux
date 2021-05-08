#ifndef _LINUX_BENCHMARK_H
#define _LINUX_BENCHMARK_H

#include <linux/timekeeping.h>

/*
    An trace struct is created when sctrace_add system call is 
    invoked. The trace is in the form of a linked list where 
    each node contains a trace ID, normal time, temperature, memory of 
    process, and TA time when the system call was invoked

    - trace ID: passed through system call
    - TA time: passed through system call
    - normal time: obtained here within the system call function
    - temperature: obtained here within the system call function
    - memory: obtained here within the system call function
*/
typedef struct sctrace {
    unsigned long id;
    struct timespec64 ts;
    struct sctrace* next;
} sctrace_t;


long add_sctrace(unsigned long id, unsigned long delta, unsigned int allocated);

long sys_get_sctrace(unsigned long return_trace);

long reset_sctrace(void);


#endif /* _LINUX_BENCHMARK_H */
