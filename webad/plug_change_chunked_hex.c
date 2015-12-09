#include "main.h"

PRIVATE int change_chunked_hex(void *data)
{
	struct http_conntrack* httpc = (struct http_conntrack *)data;
	struct _skb *skb=httpc->skb;
	if(httpc->insert_js_tag == ERROR)
	{
		return ERROR;
	}
	
    if(!skb->http_data)
	{
		return ERROR;
    }
	
    if(skb->hhdr.res_type!=HTTP_RESPONSE_TYPE_CHUNKED)
	{
		return ERROR;
    }
	
	debug_log("````````````%d---------%s\n````````````````````%s\n`````````````````\n" ,skb->http_len, skb->http_head ,skb->http_data);
    return OK;
}

int init_change_chunked_hex()
{
	new_plug(change_chunked_hex , PLUG_TYPE_RESPONSE);
	return 0;
}

