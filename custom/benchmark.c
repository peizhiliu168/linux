#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/benchmark.h>

int debug = 1;

sctrace_t* syscall_trace;

/*
    adds a new trace into the syscall_trace
    datastructure
*/
long add_sctrace(unsigned long id, unsigned long delta, unsigned int allocated)
{
    sctrace_t* new_trace = kmalloc(sizeof(sctrace_t), GFP_KERNEL | GFP_NOWAIT);
    if (!new_trace){
        if (debug) {printk("Unable to allocate new trace.\n");}
        return 1;
    }

    printk("id: %ld, delta: %ld, allocated: %ld\n", id, delta, allocated);

    struct timespec64 ts;
    ts = ns_to_timespec64(ktime_get_ns());

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
long sys_get_sctrace(unsigned long return_trace)
{
    if (debug) {printk("Getting trace...\n");}

    if (!syscall_trace){
        if (debug) {printk("No trace to get\n");}
        return 1;
    }

    printk("notice meeeeeeeeeeeee: %d", syscall_trace->id );

    return_trace = (unsigned long) syscall_trace;
    if(debug) {printk("Got trace.\n");}
    return 0;
}

/*
    clears all the existing traces 
*/ 
long reset_sctrace(void)
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