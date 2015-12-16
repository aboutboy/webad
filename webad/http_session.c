#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <netinet/in.h>

#include "util.h"
#include "http_session.h"

#define MAX_HTTP_SESSION_NUN 256
#define MAX_HTTP_SESSION_TIMEOUT_SEC 20
PRIVATE int http_session_num=0;

PRIVATE struct list_head http_session_list_head; 

void http_chsum(struct skb_buf* skb)
{
	struct iphdr *this_iphdr = (struct iphdr *) (skb->pload);  
	struct tcphdr *this_tcphdr = (struct tcphdr *) (skb->pload+ 4 * this_iphdr->ihl);
	this_iphdr->tot_len=htons(skb->pload_len);
	this_iphdr->check=ip_chsum(this_iphdr);
	this_tcphdr->check=tcp_chsum(this_iphdr , this_tcphdr , skb->pload_len-4 * this_iphdr->ihl);
}

int change_accept_encoding(struct http_hdr* hhdr)
{
	if(!strncasecmp(hhdr->accept_encoding.c, "Accept-Encoding: gzip" ,21))
	{
		hhdr->accept_encoding.c[0]='B';
		return OK;
	}

	return ERROR;
}

int http_request_filter(struct http_hdr* hhdr)
{	
	//maybe not http protocal
	if(hhdr->host.l <= 0)
		return ERROR;

	//not html page
	if(hhdr->accept.l <= 0)
		return ERROR;
	
	if(strncasecmp(hhdr->accept.c, "Accept: text/html" ,17)!=0)
	{
		return ERROR;
	}
	return OK;
}

int http_response_filter(struct http_hdr* hhdr)
{	
	//normal page
	if(hhdr->error_code.l <= 0)
		return ERROR;
	if(strncasecmp(hhdr->error_code.c , "HTTP/1.1 200 OK" , 15)!=0)
	{
		return ERROR;
	}
	//not html page
	if(hhdr->content_type.l <= 0)
		return ERROR;
	if(strncasecmp(hhdr->content_type.c, "Content-Type: text/html" ,23)!=0)
	{
		return ERROR;
	}
	return OK;
}


void free_http_response_list(struct http_request* httpr)
{
	struct skb_buf *cursor , *tmp;
    list_for_each_entry_safe(cursor, tmp, &httpr->http_response_head, list)
    {
		//free
	}	
}

int add2http_response_list(struct http_request* httpr, struct skb_buf* skb)
{
	struct skb_buf *new_skb = (struct skb_buf *)test_malloc(sizeof(struct skb_buf));
	memcpy(new_skb , skb , sizeof(struct skb_buf));
	la_list_add_tail(&new_skb->list , &httpr->http_response_head);

	httpr->skb_response_cache.pload= 
		(unsigned char*)test_remalloc(httpr->skb_response_cache.pload , 
		httpr->skb_response_cache.data_len+skb->data_len);

	memcpy(httpr->skb_response_cache.pload+httpr->skb_response_cache.data_len , 
		skb->pload , skb->pload_len);
	httpr->skb_response_cache.data_len+=skb->data_len;
	return 0;
}

void free_http_request(struct http_request* httpr)
{
	if(!httpr)
		return;
	free_http_response_list(httpr);
	la_list_del(&(httpr->list));
	test_free(httpr);
	http_session_num--;
}

struct http_request* new_http_request(struct tuple4* addr)
{
	struct http_request* new_httpr;
	
	new_httpr=(struct http_request*)test_malloc(sizeof(struct http_request));
	INIT_LIST_HEAD(&new_httpr->http_response_head);
	memcpy(&new_httpr->tcps.addr, addr ,sizeof(struct tuple4));
	new_httpr->tcps.last_time = get_current_sec();
	memset(&new_httpr->skb_response_cache , '\0' ,sizeof(struct skb_buf));
	memset(&new_httpr->hhdr, '\0' ,sizeof(struct http_hdr));
	la_list_add_tail(&(new_httpr->list), &http_session_list_head);
	http_session_num++;
	return new_httpr;
}

struct http_request* find_http_request(struct tuple4* addr , struct skb_buf* skb)
{
	struct http_request *cursor , *tmp;
	list_for_each_entry_safe(cursor, tmp, &http_session_list_head, list)
	{
		if(addr->sip == cursor->tcps.addr.sip &&
			addr->dip== cursor->tcps.addr.dip &&
			addr->sp == cursor->tcps.addr.sp &&
			addr->dp == cursor->tcps.addr.dp
			)
		{
			return cursor;
		}
		else if(addr->sip == cursor->tcps.addr.dip &&
			addr->dip== cursor->tcps.addr.sip &&
			addr->sp == cursor->tcps.addr.dp &&
			addr->dp == cursor->tcps.addr.sp &&
			skb->ack_seq == cursor->tcps.curr_seq)
		{
			return cursor;
		}
		
	}	
	return NULL;
}

void handle_http_session_from_cache(struct http_request* httpr)
{
	struct skb_buf *cursor , *tmp;
    list_for_each_entry_safe(cursor, tmp, &httpr->http_response_head, list)
    {
    	
	}
}

int decode_http(struct http_request* httpr , struct skb_buf *skb)
{
	int i = 0;
	char *start,*end;
	char* http_head;
	char* http_data;
	int http_len;

	http_len = skb->data_len;
	http_head = (char*)(skb->pload+(skb->pload_len - skb->data_len));
	if(!http_head)
		return ERROR;

	//////////////http_head_start///////////
	if(!strncasecmp(http_head,"POST ",5))
	{
		httpr->hhdr.http_type=HTTP_TYPE_REQUEST_POST;
		return ERROR;
	}
	else if(!strncasecmp(http_head,"GET " ,4))
	{
		httpr->hhdr.http_type=HTTP_TYPE_REQUEST_GET;
	}
	else if(!strncasecmp(http_head,"HTTP/1.",7))
	{
			
		httpr->hhdr.http_type=HTTP_TYPE_RESPONSE;
	}
	else 
	{
		httpr->hhdr.http_type=HTTP_TYPE_OTHER;
		return OK;
	}
	
	end=start=http_head;
	while(i<http_len)
	{	
		if(memcmp(end , "\r\n" , 2)!=0 )
		{
			i++;
			end++;
			continue;
		}

		if(!strncasecmp(start,"GET " ,4))
		{
			new_string(&httpr->hhdr.uri, start , end-start);
		}
		else if(!strncasecmp(start,"HTTP/1.",7))
		{
			new_string(&httpr->hhdr.error_code , start , end-start);
		}
		else if(!strncasecmp(start,"Host: ",6))
		{
			new_string(&httpr->hhdr.host , start , end-start);
		}
		else if(!strncasecmp(start,"Accept-Encoding: ",17))
		{
			new_string(&httpr->hhdr.accept_encoding, start , end-start);
		}
		else if(!strncasecmp(start,"Accept: ",8))
		{
			new_string(&httpr->hhdr.accept, start , end-start);
		}
		else if(!strncasecmp(start,"User_Agent: ",12))
		{
			new_string(&httpr->hhdr.user_agent, start , end-start);
		}
		else if(!strncasecmp(start,"Content-Type: ",14))
		{
			new_string(&httpr->hhdr.content_type, start , end-start);
		}
		else if(!strncasecmp(start,"Content_Encoding: ",18))
		{
			new_string(&httpr->hhdr.content_encoding, start , end-start);
		}
		else if(!strncasecmp(start,"Content-Length: ",16))
		{
			new_string(&httpr->hhdr.content_length, start , end-start);
			httpr->hhdr.res_type=HTTP_RESPONSE_TYPE_CONTENTLENGTH;
		}
		else if(!strncasecmp(start,"Transfer-Encoding: chunked",26))
		{
			new_string(&httpr->hhdr.transfer_encoding, start , end-start);
			httpr->hhdr.res_type=HTTP_RESPONSE_TYPE_CHUNKED;
		}
		i+=2;
		end+=2;
		start=end;
		if(!memcmp(start , "\r\n" , 2))
		{
			start+=2;
			break;
		}
		
	}

	//////////////http_head_end///////////
	http_data=start;
	if(!http_data)
	{
		return ERROR;
	}
	
	//////////////include /r/n/r/n ///////
	httpr->hhdr.httph_len = http_data - http_head;
	
	return OK;
}

void process_http(struct skb_buf *skb ,void (*callback)(void*))
{
	struct http_request* new_httpr;
	struct tuple4 addr;
	
	struct iphdr *this_iphdr = (struct iphdr *) (skb->pload);  
	struct tcphdr *this_tcphdr = (struct tcphdr *) (skb->pload+ 4 * this_iphdr->ihl);
	
	addr.sip = this_iphdr->saddr;
	addr.dip = this_iphdr->daddr;
	addr.sp = this_tcphdr->source;
	addr.dp = this_tcphdr->dest;

	new_httpr=find_http_request(&addr ,skb);
	switch(skb->result)
	{
		case RESULT_FROM_CLIENT:
			{
				//first request packet must get
				if(!new_httpr)
				{
					new_httpr = new_http_request(&addr);
					if(ERROR == decode_http(new_httpr , skb))
					{
						goto result_ignore;
					}

					if(new_httpr->hhdr.http_type != HTTP_TYPE_REQUEST_GET)
					{
						goto result_ignore;
					}
					
					if(ERROR == http_request_filter(&new_httpr->hhdr))
					{
						goto result_ignore;
					}
					if(ERROR == change_accept_encoding(&new_httpr->hhdr))
					{
						goto result_ignore;
					}
					
					http_chsum(skb);
					
					new_httpr->tcps.curr_seq = skb->seq + skb->data_len;
					new_httpr->tcps.callback = callback;
					callback(skb);
					return;
				}
				
				//repeat session request
				goto result_ignore;
			}
		case RESULT_FROM_SERVER:
			{
				//no find
				if(!new_httpr)
				{
					goto result_ignore;
				}
			
				//first response packet must have http head
				if(new_httpr->hhdr.http_type == HTTP_TYPE_REQUEST_GET)
				{
					if(ERROR == decode_http(new_httpr , skb))
					{
						goto result_ignore;
					}
					if(new_httpr->hhdr.http_type != HTTP_TYPE_RESPONSE)
					{
						goto result_ignore;
					}

					if(ERROR == http_response_filter(&new_httpr->hhdr))
					{
						goto result_ignore;
					}
					//add2http_response_list(new_httpr , skb);
					callback(skb);
					return;
				}
				//other response packet no need decode http head
				if(new_httpr->hhdr.http_type == HTTP_TYPE_RESPONSE)
				{
					//add2http_response_list(new_httpr , skb);
					//handle_http_session_from_cache(new_httpr);
					callback(skb);
					return;
				}
				else
				{
					goto result_ignore;
				}
				break;
			}
			
		default:break;	
	}

	result_ignore:
		free_http_request(new_httpr);
		skb->result=RESULT_IGNORE;
		callback(skb);
		return;
}

void init_http_session()
{
	INIT_LIST_HEAD(&http_session_list_head);
}

