
#include "main.h"

PRIVATE int change_url(void *data)
{
	
	return OK;
}

int init_change_url()
{
	new_plug(change_url , PLUG_TYPE_GET);
	return OK;
}
