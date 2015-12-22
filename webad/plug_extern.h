#ifndef __PLUG_EXTERN_H__
#define __PLUG_EXTERN_H__

//see file plug.h PLUG_TYPE_MAX
typedef enum 
{
	PLUG_EXTERN_TYPE_OTHER=0,
	PLUG_EXTERN_TYPE_REQUEST,
	PLUG_EXTERN_TYPE_RESPONSE
	
}PLUG_EXTERN_TYPE;

int init_plug_extern();

#endif 

