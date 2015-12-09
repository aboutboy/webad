
#include "main.h"

PRIVATE int change_url(void *data)
{
	
	return OK;
}

int init_change_url()
{
	new_check_plug(change_url , CHECK_PLUG_PRE);
	return OK;
}
