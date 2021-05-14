#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/benchmark.h>

#define DEBUG 1

sctrace_t* syscall_trace;

/*
    adds a new trace into the syscall_trace
    datastructure
*/
long add_sctrace(unsigned long id, unsigned long delta, unsigned int allocated)
{
    long ree_time = ns_to_timespec64(ktime_get_ns()).tv_nsec;

    if (DEBUG) {printk("Adding trace...\n");}
    
    sctrace_t* new_trace = kmalloc(sizeof(sctrace_t), GFP_KERNEL | GFP_NOWAIT);
    if (!new_trace){
        if (DEBUG) {printk("Unable to allocate new trace.\n");}
        return 1;
    }

    new_trace->id = id;
    new_trace->delta = delta;
    new_trace->allocated = allocated;
    new_trace->ree_time = ree_time;
    new_trace->next = syscall_trace;
    
    // memcpy((void*) &new_trace->id, (void*) &id, sizeof(unsigned long));
    // memcpy((void*) &new_trace->delta, (void*) &id, sizeof(unsigned long));
    // memcpy((void*) &new_trace->allocated, (void*) &id, sizeof(unsigned long));
    // memcpy((void*) &new_trace->ree_time, (void*) &ree_time, sizeof(long));

    syscall_trace = new_trace;
    if (DEBUG) {printk("Successfully added trace.\n");}
    return 0;
}

/*
    takes in a sctrace_t pointer to set with the trace.
    If no trace exists, return 1
*/
long sys_get_sctrace(unsigned long return_trace)
{
    if (DEBUG) {printk("Getting trace...\n");}

    if (!syscall_trace){
        if (DEBUG) {printk("No trace to get\n");}
        return 1;
    }

    sctrace_t* temp = syscall_trace;

    while (syscall_trace) {
        printk("id: %ld, delta: %ld, allocated: %ld, reetime: %ld\n", temp->id, temp->delta, temp->allocated, temp->ree_time);
        temp = temp->next;
    }

    return_trace = (unsigned long) syscall_trace;
    if(DEBUG) {printk("Got trace.\n");}
    return 0;
}

/*
    clears all the existing traces 
*/ 
long reset_sctrace(void)
{
    if(DEBUG) {printk("Clearing existing traces...\n");}
    
    if (!syscall_trace){
        return 0;
    }
    
    while (syscall_trace) {
        sctrace_t* temp = syscall_trace->next;
        kfree(syscall_trace);
        syscall_trace = temp;
    }

    if (DEBUG) {printk("Existing traces cleared.\n");}

    return 0;
}