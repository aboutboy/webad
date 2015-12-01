#include "main.h"


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
**task_shm_id	数据共享任务号
				表示当前任务需要读取这个任务号对应的共享数据
**task_num 		任务数量
**task			任务主函数
*****************/
struct task_info* new_task(int task_id , 
	int task_shm_id ,
	int task_num, 
	void (*task)(struct task_info*))
{

	struct task_info* new;
	if(task_num>MAX_ONE_CREATE_PID_NUM)
	{
		debug_log("max task_num arrivte %d",task_num);
		return NULL;
	}
	if(!task_list)
	{
		task_list=(struct list_head*)new_page(sizeof(struct list_head));
		if(!task_list)
			return NULL;
		INIT_LIST_HEAD(task_list);
	}
	
	new=(struct task_info*)new_page(sizeof(struct task_info));
	if(!new)
		return NULL;
	
	new->task_id=task_id;
	new->task_shm_id=task_shm_id;
	new->task_num=task_num;
	new->pid_num=task_num;
	new->task=task;
	
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
		if((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		{
		       debug_log("task_manage child %d terminated", pid);
			   ti=task_find_by_pid(pid);
			   if(ti)
			   {
			   		ti->task_num--;
					debug_log("recreate task task_id %d task_num %d(0:recreate,>0:no)", 
							ti->task_id,ti->task_num);
			   		if(ti->task_num==0)
					{
						ti->task_num=ti->pid_num;
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
	char data[32]="task1";
	int len=strlen(data);
	ti->task_shm_queue(ti , TASK_TYPE_WRITE , data , &len);
	sleep(100);
}

PRIVATE void task2(struct task_info *ti)
{
	debug_log("this is task2");
	char data[32]={0};
	int len;
	ti->task_shm_queue(ti , TASK_TYPE_READ , data , &len);
	debug_log("task2 read shm queue data %s len %d" , data , len);
	sleep(100);
}

int main()
{
	int t1=1,t2=2;
	new_task(t1, 0, 1, task1);
	new_task(t2, t1, 1, task2);
	free_task();
	
	return OK;
}
*/

