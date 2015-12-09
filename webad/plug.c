
#include "main.h"
#include "plug_change_url.h"
#include "plug_insert_js.h"
#include "plug_redirect_url.h"
#include "plug_change_accept_encoding.h"
#include "plug_change_seq.h"

struct plug_info
{
	struct list_head list;
	PLUG_TYPE type;
	int (*plug)(void*);
};

PRIVATE struct list_head* plug_list;

void new_plug(int (*plug_hook)(void *) , PLUG_TYPE type)
{

	struct plug_info* new;

	new=(struct plug_info*)new_page(sizeof(struct plug_info));
	if(!new)
		return;
	
	new->type=type;
	new->plug=plug_hook;
	la_list_add_tail(&(new->list), plug_list);
	
}

int plug_hook(void *data , PLUG_TYPE type)
{
	struct plug_info *pi,*tmp;
	
	list_for_each_entry_safe(pi, tmp, plug_list, list) 
	{
		if(type == pi->type)
			pi->plug(data);
	}
	return OK;
}

int init_plug()
{

	plug_list=(struct list_head*)new_page(sizeof(struct list_head));
	if(!plug_list)
		return -1;
	
	INIT_LIST_HEAD(plug_list);
	
	init_change_accept_encoding();
	//init_change_url();
	//init_redirect_url();
	init_insert_js();
	init_change_seq();
	return 0;
}

void fini_plug()
{

}

