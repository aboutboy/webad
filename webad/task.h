
#ifndef __TASK_H__
#define __TASK_H__
#include <sys/wait.h>
#include <semaphore.h>

#include "list.h"

#define MAX_DATA_LEN  2048
#define MAX_ONE_CREATE_PID_NUM   8

enum task_type{
	TASK_TYPE_OTHER,
	TASK_TYPE_READ,
	TASK_TYPE_WRITE
};

enum task_action{
	TASK_ACTION_DEFAULT,
	TASK_ACTION_KILL,
	TASK_ACTION_STOP,
	TASK_ACTION_CONTINUE
};



struct _task_shm_queue
{
	struct list_head list;
	int len;
	char data[MAX_DATA_LEN];
};

struct task_info
{
	struct list_head list;
	int task_id;
	int task_shm_id;
	int task_num;
	int pid_num;
	int pid[MAX_ONE_CREATE_PID_NUM];
	void (*task)(struct task_info*);
	sem_t mutex,full,empty;
	struct _task_shm_queue *front,*rear;
	struct _task_shm_queue tsq;
	int (*task_shm_queue)(struct task_info* ,int , void* ,int*);
};

int get_cpu_num();

int bind_cpu(int* cpu_num);

struct task_info* new_task(int task_id ,
	int task_shm_id ,
	int task_num,
	void (*task)(struct task_info*));

void task_manage();
void task_action(int task_id ,int action_type);
void signal_ignore();

#endif
