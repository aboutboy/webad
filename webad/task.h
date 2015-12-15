
#ifndef __TASK_H__
#define __TASK_H__
#include <sys/wait.h>

#include "list.h"

#define MAX_ONE_CREATE_PID_NUM   8

enum task_action{
	TASK_ACTION_DEFAULT,
	TASK_ACTION_KILL,
	TASK_ACTION_STOP,
	TASK_ACTION_CONTINUE
};

struct task_info
{
	struct list_head list;
	int task_id;
	int pid_num;
	int pid[MAX_ONE_CREATE_PID_NUM];
	void (*task)(struct task_info*);
};

int get_cpu_num();

struct task_info* new_task(int task_id ,
	int task_num,
	void (*task)(struct task_info*));

void task_manage();
void task_action(int task_id ,int action_type);
void signal_ignore();

#endif
