#ifndef __PLUG_H__
#define __PLUG_H__

typedef enum 
{
	PLUG_TYPE_GET,
	PLUG_TYPE_RESPONSE,
	PLUG_TYPE_OTHER,
	PLUG_TYPE_MAX
}PLUG_TYPE;

void new_plug(int (*plug_hook)(void *) , PLUG_TYPE type);

int plug_hook(void *data , PLUG_TYPE type);

int init_plug();
void fini_plug();

#endif 
