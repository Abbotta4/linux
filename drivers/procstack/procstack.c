// SPDX-License-Identifier: MIT
/*
 * procstack - A simple driver for pushing text to a stack
 * Copyright (C) 2023 Drew Abbott <abbotta4@gmail.com>
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>

#define BUFF_SIZE 100

struct list_node {
	char* data;
	int len; // length excluding null-terminator
	struct list_node* next;
};

struct list_node* g_HEAD = NULL;

static int procstack_show(struct seq_file *m, void *v)
{
	struct list_node* cur = g_HEAD;
	char* buf = kmalloc(BUFF_SIZE, GFP_KERNEL);

	while (cur) {
		int len = strlen(buf) + cur->len + 1 <= BUFF_SIZE ? cur->len : BUFF_SIZE - strlen(buf) - 1;
		if (len > 0) {
			strncat(buf, cur->data, len);
		}
		else {
			break;
		}
		cur = cur->next;
	}
	seq_printf(m, "%s", buf);

	return 0;
}

static int procstack_open(struct inode *inode, struct file *file)
{
	return single_open(file, procstack_show, NULL);
}

static ssize_t procstack_write(struct file *file, const char __user *buf,
							   size_t count, loff_t* offset)
{
	struct list_node* cur = kmalloc(sizeof(struct list_node), GFP_KERNEL);

	cur->data = (char*)memdup_user_nul(buf, count);
	cur->next = g_HEAD;
	cur->len = strlen(cur->data);
	g_HEAD = cur;

	return count;
}

static const struct proc_ops procstack_ops = {
	.proc_open	 = procstack_open,
	.proc_read	 = seq_read,
	.proc_write	 = procstack_write,
};

static int __init hello_init(void) {
	if (!proc_create("procstack", 0666, NULL, &procstack_ops))
		pr_warn("procstack: unable to create proc entry\n");
	return 0;
}

static void __exit hello_exit(void) {
	struct list_node* cur = g_HEAD;
	struct list_node* tmp = NULL;

	remove_proc_entry("procstack", NULL);
	while (cur) {
		tmp = cur;
		cur = cur->next;
		kfree(tmp);
	}
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("MIT");
MODULE_AUTHOR("Drew Abbott");
MODULE_DESCRIPTION("A simple driver for pushing and popping text");
