#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/benchmark.h>

int debug = 1;

sctrace_t* syscall_trace;

/*

*/
SYSCALL_DEFINE1(add_sctrace, unsigned long, id)
{
    sctrace_t* new_trace = kmalloc(sizeof(sctrace_t), GFP_KERNEL | GFP_NOWAIT);
    if (!new_trace){
        if (debug) {printk("Unable to allocate new trace.\n");}
        return 1;
    }

    struct timespec64 ts = ns_to_timespec64(ktime_get_ns());

    memcpy((void*) &new_trace->id, (void*) &id, sizeof(unsigned long));
    memcpy((void*) &new_trace->ts, (void*) &ts, sizeof(struct timespec64));

    new_trace->next = syscall_trace;
    syscall_trace = new_trace;
    return 0;
}

/*
    takes in a sctrace_t pointer to set with the trace.
    If no trace exists, return 1
*/
SYSCALL_DEFINE1(get_sctrace, sctrace_t*, return_trace)
{
    if (debug) {printk("Getting trace...\n");}
    (sctrace_t*) return_trace;

    if (!syscall_trace){
        if (debug) {printk("No trace to get\n");}
        return 1;
    }

    return_trace = syscall_trace;
    if(debug) {printk("Got trace.\n");}
    return 0;
}

/*
    clears all the existing traces 
*/ 
SYSCALL_DEFINE0(reset_sctrace)
{
    if(debug) {printk("Clearing existing traces...\n");}
    
    if (!syscall_trace){
        return 0;
    }
    
    while (1) {
        if (syscall_trace == NULL){
            break;
        }   
        sctrace_t* temp = syscall_trace->next;
        kfree(syscall_trace);
        syscall_trace = temp;
    }

    if (debug) {printk("Existing traces cleared.\n");}

    return 0;
}