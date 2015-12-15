#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h> 

#include "util.h"
#include "task.h"


PRIVATE struct list_head* task_list=NULL;

int get_cpu_num()
{
	return sysconf(_SC_NPROCESSORS_CONF);
}

PRIVATE int task_create(struct task_info* ti)
{
	pid_t pid=0;
	int i;
	if(!ti)
		return ERROR;
	
	for(i=0;i<ti->pid_num;i++)
	{
		pid=fork();
		switch(pid)
		{
			case -1:
				debug_log("fork error");
				return -1;
			case 0:
				ti->task(ti);
				return OK;
			default:
				ti->pid[i]=pid;
				debug_log("Child's pid is %d\n",pid);
				break;
		}
	}
	return pid;
}

/*****************
**task_id 		当前任务号
**pid_num 		任务数量
**task			任务主函数
*****************/
struct task_info* new_task(int task_id , 
	int pid_num, 
	void (*task)(struct task_info*))
{

	struct task_info* new;
	if(pid_num>MAX_ONE_CREATE_PID_NUM)
	{
		debug_log("max task_num arrivte %d",pid_num);
		return NULL;
	}
	if(!task_list)
	{
		task_list=(struct list_head*)malloc(sizeof(struct list_head));
		if(!task_list)
			return NULL;
		INIT_LIST_HEAD(task_list);
	}
	
	new=(struct task_info*)malloc(sizeof(struct task_info));
	if(!new)
		return NULL;
	
	new->task_id=task_id;
	new->pid_num=pid_num;
	new->task=task;
	la_list_add_tail(&(new->list), task_list);
	task_create(new);
	return new;
	
}

PRIVATE struct task_info* task_find_by_pid(int pid)
{
	struct task_info *ti,*tmp;
	int i;
	list_for_each_entry_safe(ti, tmp, task_list, list) 
	{
		for(i=0;i<ti->pid_num;i++)
		{
			if(ti->pid[i] == pid)
			{
				return ti;
			}
		}
	}
	
	return NULL;
}

void task_manage()
{
	
	pid_t pid;
	int stat;
	struct task_info* ti;
	
	while(1)
	{
		if((pid = waitpid(-1, &stat, WNOHANG | WUNTRACED)) > 0)
		{
			debug_log("task_manage child %d terminated", pid);
			ti=task_find_by_pid(pid);
			if(!ti)
			{
				debug_log("task_manage child %d can not find", pid);
				continue;
			}
			if(WIFSTOPPED(stat))
			{
				task_action(ti->task_id ,TASK_ACTION_CONTINUE);
			}
			else if(WIFEXITED(stat)||WIFSIGNALED(stat))
			{
				
				ti->pid_num--;
				debug_log("recreate task task_id %d task_num %d(0:recreate,>0:no)", 
					ti->task_id,ti->pid_num);
				if(ti->pid_num==0)
				{
					ti->pid_num++;
					task_create(ti);
				}
			}   
		} 
		sleep(2);
	}
}


PRIVATE struct task_info* task_find_by_task_id(int task_id)
{
	struct task_info *ti,*tmp;
	list_for_each_entry_safe(ti, tmp, task_list, list) 
	{
		if(ti->task_id== task_id)
		{
			return ti;
		}
		
	}
	
	return NULL;
}

void task_action(int task_id ,int action_type)
{
	int i=0;
	struct task_info *ti;
	ti=task_find_by_task_id(task_id);
	if(!ti)
		return;
	
	while(i<ti->pid_num)
	{
		switch(action_type)
		{
			case TASK_ACTION_KILL:
				debug_log("kill task pid %d",ti->pid[i]);
				kill(ti->pid[i], SIGTERM);	
				break;
			case TASK_ACTION_STOP:
				debug_log("stop task pid %d",ti->pid[i]);
				kill(ti->pid[i], SIGSTOP);
				break;
			case TASK_ACTION_CONTINUE:
				debug_log("continue task pid %d",ti->pid[i]);
				kill(ti->pid[i], SIGCONT);
				break;
			default:
				break;
		}
		i++;
	}
}

void signal_ignore()
{
	signal(SIGHUP, SIG_IGN); //session
	signal(SIGPIPE, SIG_IGN); //pipe
	signal(SIGCHLD, SIG_IGN);//fork exit child > father
	//signal(SIGTERM, SIG_IGN);//kill pid
}


/*

PRIVATE void task1(struct task_info *ti)
{
	debug_log("this is task1");
	sleep(100);
}

PRIVATE void task2(struct task_info *ti)
{
	debug_log("this is task2");
	sleep(100);
}

int main()
{
	int t1=1,t2=2;
	new_task(t1, 1, task1);
	new_task(t2, 1, task2);
	
	return OK;
}
*/

