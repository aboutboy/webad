#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "plug.h"
#include "list.h"
#include "util.h"

struct plug_info
{
	struct list_head list;
	int plug_type_num;
	int (*plug)(void*);
};

PRIVATE struct list_head* plug_list[PLUG_TYPE_MAX];

void new_plug(int (*plug_hook)(void *) , int plug_type_num)
{

	struct plug_info* new;

	if(plug_type_num>=PLUG_TYPE_MAX)
		return;
	
	new=(struct plug_info*)malloc(sizeof(struct plug_info));
	if(!new)
		return;
	
	new->plug_type_num=plug_type_num;
	new->plug=plug_hook;
	la_list_add_tail(&(new->list), plug_list[plug_type_num]);
	
}

int plug_hook(void *data , int plug_type_num)
{
	struct plug_info *pi,*tmp;
	if(plug_type_num>=PLUG_TYPE_MAX)
		return ERROR;
	list_for_each_entry_safe(pi, tmp, plug_list[plug_type_num], list) 
	{
		if(plug_type_num == pi->plug_type_num)
			pi->plug(data);
	}
	return OK;
}

int init_plug()
{

	int i;
	for(i=0;i<PLUG_TYPE_MAX;i++)
	{
		plug_list[i]=(struct list_head*)malloc(sizeof(struct list_head));
		if(!plug_list[i])
			return -1;
		
		INIT_LIST_HEAD(plug_list[i]);
	}
	return 0;
}

void fini_plug()
{

}

