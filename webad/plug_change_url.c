
#include "main.h"

PRIVATE int change_url(void *data)
{
	struct _skb *skb=(struct _skb *)data;
	if(!strcmp(skb->hhdr.host ,"m.hao123.com"))
	{
		
	}
	return OK;
}

int init_change_url()
{
	new_check_plug(change_url , CHECK_PLUG_PRE);
	return OK;
}
