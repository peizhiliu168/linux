#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/benchmark.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define DEBUG 1
#define PATH "/tmp/trace_%lu.csv" // base file name containing id suffix

/*
    Wrapper to open file in kernel mode, file
    must be created within existing directory.
    Takes a path and returns a file pointer.
*/
struct file* open_file(const char *path)
{
    struct file* filp;
    mm_segment_t fs;
    int err;
    int flags;

    // We must expand the memory range to include kernel memory area
    fs = get_fs();
    set_fs(KERNEL_DS);

    flags = O_CREAT | O_RDWR; // create a new file and make it r/w
    filp = filp_open(path, flags, 0666);

    set_fs(fs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        printk("error creating file...");
        return NULL;
    }

    return filp;
}

/*
    Closes the file given by the file pointer.
*/
void close_file(struct file* filp)
{
    filp_close(filp, NULL);
}

/*
    Reads the contents of the file and writes it 
    into the given buffer. Amount read and offset
    in file is specified by count and pos respect-
    ively.
*/
int read_file(struct file *file, void *buf, size_t count, loff_t *pos)
{
    mm_segment_t fs;
    int ret;

    fs = get_fs();
    set_fs(KERNEL_DS);

    ret = kernel_read(file, buf, count, pos);

    set_fs(fs);
    return ret;
}

/*
    Similar to read but opposite. Writes the 
    contents of the buffer into the file. Amount
    read and offset in file is specified by count
    and pos respectively.
*/
int write_file(struct file *file, const void *buf, size_t count, loff_t *pos)
{
    mm_segment_t fs;
    int ret;

    fs = get_fs();
    set_fs(KERNEL_DS);

    ret = kernel_write(file, buf, count, pos);

    set_fs(fs);
    return ret;
}

// head of doubly-linked list string traces
sctrace_t* syscall_trace;

/*
    adds a new trace into the syscall_trace
    datastructure
*/
long add_sctrace(unsigned long id, unsigned long delta, unsigned int allocated)
{
    long ree_time;
    sctrace_t* new_trace;

    if (!syscall_trace) {
        syscall_trace = kmalloc(sizeof(sctrace_t), GFP_KERNEL | GFP_NOWAIT);
        syscall_trace->next = syscall_trace;
        syscall_trace->prev = syscall_trace;
    }
        
    ree_time = ns_to_timespec64(ktime_get_ns()).tv_nsec;

    if (DEBUG) {printk("Adding trace...\n");}
    
    new_trace = kmalloc(sizeof(sctrace_t), GFP_KERNEL | GFP_NOWAIT);
    if (!new_trace){
        if (DEBUG) {printk("Unable to allocate new trace.\n");}
        return 1;
    }

    new_trace->id = id;
    new_trace->delta = delta;
    new_trace->allocated = allocated;
    new_trace->ree_time = ree_time;
    
    // memcpy((void*) &new_trace->id, (void*) &id, sizeof(unsigned long));
    // memcpy((void*) &new_trace->delta, (void*) &id, sizeof(unsigned long));
    // memcpy((void*) &new_trace->allocated, (void*) &id, sizeof(unsigned long));
    // memcpy((void*) &new_trace->ree_time, (void*) &ree_time, sizeof(long));
    
    syscall_trace->next->prev = new_trace;
    new_trace->next = syscall_trace->next;
    new_trace->prev = syscall_trace;
    syscall_trace->next = new_trace;
    if (DEBUG) {printk("Successfully added trace.\n");}
    return 0;
}

/*
    takes in a sctrace_t pointer to set with the trace.
    If no trace exists, return 1
*/
long get_sctrace(unsigned long return_trace)
{
    if (DEBUG) {printk("Getting trace...\n");}
    
    sctrace_t* temp;
    struct file* filp;
    loff_t loff;
    char fields[] = "id, delta, allocated, reetime\n";
    char template[] = "%lu, %lu, %lu, %ld\n";
    char buffer[128];
    char path[128];
    
    if (!syscall_trace){
        if (DEBUG) {printk("No trace to get\n");}
        return 1;
    }

    temp = syscall_trace->prev;
    loff = 0;
    snprintf(path, sizeof(path), PATH, return_trace);
    
    // open and write to file
    filp = open_file(path);
    if (DEBUG) {printk("opened file at %x\n", filp);}
    write_file(filp, fields, strlen(fields), &loff);
    if (DEBUG) {printk("wrote header: %s", fields);}
    
    // populate written buffer
    while (temp != syscall_trace) {
        if (DEBUG) {printk("%lu, %lu, %lu, %ld, location: %ld\n", temp->id, temp->delta, temp->allocated, temp->ree_time, loff);}

        snprintf(buffer, sizeof(buffer), template, temp->id, temp->delta, temp->allocated, temp->ree_time);

        write_file(filp, buffer, strlen(buffer), &loff);
        
        if (DEBUG) {printk("len: %lu", strlen(buffer));}

        temp = temp->prev;
    }

    close_file(filp);

    return_trace = (unsigned long) syscall_trace;
    if(DEBUG) {printk("Got trace.\n");}
    return 0;
}

/*
    clears all the existing traces 
*/ 
long reset_sctrace(void)
{    
    sctrace_t* temp;

    if(DEBUG) {printk("Clearing existing traces...\n");}
    
    if (!syscall_trace){
        return 0;
    }

    while (syscall_trace->next != syscall_trace) {
        temp = syscall_trace->next;
        syscall_trace->next = temp->next;
        kfree(temp);
    }

    if (DEBUG) {printk("Existing traces cleared.\n");}

    return 0;
}