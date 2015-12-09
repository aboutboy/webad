#include "main.h"

PRIVATE int change_chunked_hex(void *data)
{
	struct http_conntrack* httpc = (struct http_conntrack *)data;
	struct _skb *skb=httpc->skb;
	char *hex_start,*hex_end;
	char src_hex[8]={0} ,des_hex[8]={0};
	int hex_len;
	int hex_i;
	
	//after insert_js
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
	hex_start = skb->http_data;
	hex_end = strstr(hex_start , "\r\n");
	if(!hex_end)
	{
		return ERROR;
	}

	hex_len=hex_end-hex_start;
	
	memcpy(src_hex , hex_start , hex_len);
	if(hex_len>4)
	{
		return ERROR;
	}
	hex2i(src_hex, &hex_i);
	hex_i+=httpc->insert_js_len;
	i2hex(hex_i, des_hex);
	if(hex_len!=strlen(des_hex))
	{
		return ERROR;
	}
	memcpy(hex_start , des_hex , hex_len);
	skb->tcp->check=tcp_chsum(skb->iph , skb->tcp , skb->tcp_len);
	//debug_log("````````````%d---------%s\n````````````````````%s\n`````````````````\n" ,skb->http_len, skb->http_head ,skb->http_data);

	return OK;
}

int init_change_chunked_hex()
{
	new_plug(change_chunked_hex , PLUG_TYPE_RESPONSE);
	return 0;
}

