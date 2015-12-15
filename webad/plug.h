#ifndef __PLUG_H__
#define __PLUG_H__

#define PLUG_TYPE_MAX 10

void new_plug(int (*plug_hook)(void *) , int plug_type_num);

int plug_hook(void *data , int plug_type_num);

int init_plug();
void fini_plug();

#endif 
