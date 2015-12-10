#include "main.h"

#define TIMEOUT_HTTP 60*60

struct list_head httpc_list;


///////////////////////////////////////////////////////////////////////

void timeout(void* arg)
{
	struct http_conntrack *httpc_cursor , *httpc_tmp;
	
	long current_sec;
	
	while(1)
	{
		
		sleep(TIMEOUT_HTTP);
		current_sec=get_current_sec();

		list_for_each_entry_safe(httpc_cursor, httpc_tmp, &httpc_list, list)
		{		
			if(current_sec - httpc_cursor->last_time > TIMEOUT_HTTP)
			{
				debug_log("httpc timeout");
				thread_lock();
				la_list_del(&httpc_cursor->list);
				free_page(httpc_cursor);
				httpc_cursor=NULL;
				thread_unlock();
				continue;
			}
			
		}	
	}
}
///////////////////////////////////////////////////////////////////////

struct http_conntrack* find_http_conntrack_by_host(struct _skb *skb)
{
	struct http_conntrack *cursor , *tmp;
	list_for_each_entry_safe(cursor, tmp, &httpc_list, list)
	{
		if(!memcmp(skb->hhdr.host.c , cursor->host , skb->hhdr.host.l) ||
			(skb->iph->saddr == cursor->sip &&
			skb->iph->daddr == cursor->dip))
		{
			return cursor;
		}
	}	
	return NULL;
}

struct http_conntrack* find_http_conntrack_by_ack(struct _skb *skb)
{
	struct http_conntrack *cursor , *tmp;
	
	list_for_each_entry_safe(cursor, tmp, &(httpc_list), list)
	{		
		if((skb->iph->saddr == cursor->dip &&
			skb->iph->daddr == cursor->sip) &&
			ntohl(skb->tcp->ack_seq) == ntohl(cursor->seq)+cursor->http_len)
		{
			return cursor;
		}
	}	
	return NULL;
}

struct http_conntrack* init_httpc(struct _skb *skb)
{
	struct http_conntrack *httpc;
	httpc=(struct http_conntrack*)new_page(sizeof(struct  http_conntrack));
	if(!httpc)
	{
		return NULL;
	}
	thread_lock();	
	httpc->last_time = get_current_sec();
	httpc->sip = skb->iph->saddr;
	httpc->dip = skb->iph->daddr;
	httpc->seq = skb->tcp->seq;
	httpc->ack_seq = skb->tcp->ack_seq;
	httpc->http_len = skb->http_len;
	httpc->insert_js_tag=ERROR;
	httpc->insert_js_len=0;
	httpc->insert_js_seq=0;
	httpc->skb = skb;
	if(skb->hhdr.host.l < COMM_MAX_LEN)
		strncpy(httpc->host , skb->hhdr.host.c ,skb->hhdr.host.l);
	
	la_list_add_tail(&(httpc->list), &(httpc_list));
	thread_unlock();
	return httpc;
}

int update_httpc(struct http_conntrack *httpc,
	struct _skb *skb)
{
	thread_lock();
	httpc->last_time = get_current_sec();
	httpc->seq = skb->tcp->seq;
	httpc->ack_seq = skb->tcp->ack_seq;
	httpc->http_len = skb->http_len;
	httpc->insert_js_tag=ERROR;
	httpc->insert_js_len=0;
	httpc->insert_js_seq=0;
	httpc->skb = skb;
	thread_unlock();
	return 0;
}

int get_filter(struct _skb* skb)
{	
	//maybe not http protocal
	if(skb->hhdr.host.l <= 0)
		return ERROR;

	//not html page
	if(skb->hhdr.accept.l <= 0)
		return ERROR;
	
	if(strncasecmp(skb->hhdr.accept.c, "Accept: text/html" ,17)!=0)
	{
		return ERROR;
	}
	return OK;
}

int response_filter(struct _skb* skb)
{	
	//normal page
	if(skb->hhdr.error_code.l <= 0)
		return ERROR;
	if(strncasecmp(skb->hhdr.error_code.c , "HTTP/1.1 200 OK" , 15)!=0)
	{
		return ERROR;
	}
	//not html page
	if(skb->hhdr.content_type.l <= 0)
		return ERROR;
	if(strncasecmp(skb->hhdr.content_type.c, "Content-Type: text/html" ,23)!=0)
	{
		return ERROR;
	}
	return OK;
}


int dispath(struct _skb* skb)
{
	struct http_conntrack* httpc;
	//too short may be syn,fin,ack,heart packet
	if(skb->http_len <=1 || !skb->http_head )
	{
		return ERROR;
	}

	//too long MTU 1500 if MTU >1500 kernel continue subpackage 
	//if( skb->ip_len >= PKT_LEN )
	//{
	//	return ERROR;
	//}
	
	switch (skb->hhdr.http_type)
	{
		case HTTP_TYPE_REQUEST_GET:
		{
			if(ERROR == get_filter(skb))
			{
				return ERROR;
			}
			
			httpc=find_http_conntrack_by_host(skb);
			if(!httpc)
			{
				httpc=init_httpc(skb);
				if(!httpc)
				{
					return ERROR;
				}
			}
			else
			{
				update_httpc(httpc , skb);

			}
			
			plug_hook(httpc , PLUG_TYPE_GET);
			
			return OK;
			
		}
		case HTTP_TYPE_REQUEST_POST:
		{
			return ERROR;
		}
		case HTTP_TYPE_RESPONSE:	
		{
			if(ERROR == response_filter(skb))
			{
				return ERROR;
			}
			httpc=find_http_conntrack_by_ack(skb);
			if(!httpc)
			{
				return ERROR;
			}
			
			httpc->skb = skb;
			
			plug_hook(httpc , PLUG_TYPE_RESPONSE);
			
			return OK;			
		}
		case HTTP_TYPE_OTHER:
		default:
		{
			httpc=find_http_conntrack_by_ack(skb);
			if(!httpc)
			{
				return ERROR;
			}
			
			httpc->skb = skb;
			
			plug_hook(httpc , PLUG_TYPE_OTHER);
			return OK;
		}
	}
	return OK;
}

///////////////////////////////////////////////////////////////

int decode_http(struct _skb *skb)
{
	int i = 0;
	char *start,*end;
	
	skb->http_len=skb->ip_len-skb->iph_len-skb->tcph_len;
	if(skb->http_len<=0)
		return ERROR;
	
	skb->http_head=(char*)(skb->pload+skb->iph_len+skb->tcph_len);
	if(!skb->http_head)
		return ERROR;

	//////////////http_head_start///////////
	if(!strncasecmp(skb->http_head,"POST ",5))
	{
		skb->hhdr.http_type=HTTP_TYPE_REQUEST_POST;
		return ERROR;
	}
	else if(!strncasecmp(skb->http_head,"GET " ,4))
	{
		skb->hhdr.http_type=HTTP_TYPE_REQUEST_GET;
	}
	else if(!strncasecmp(skb->http_head,"HTTP/1.",7))
	{
			
		skb->hhdr.http_type=HTTP_TYPE_RESPONSE;
	}
	else 
	{
		skb->hhdr.http_type=HTTP_TYPE_OTHER;
		return OK;
	}
	
	end=start=skb->http_head;
	while(i<skb->http_len)
	{	
		if(memcmp(end , "\r\n" , 2)!=0 )
		{
			i++;
			end++;
			continue;
		}

		if(!strncasecmp(start,"GET " ,4))
		{
			new_string(&skb->hhdr.uri, start , end-start);
		}
		else if(!strncasecmp(start,"HTTP/1.",7))
		{
			new_string(&skb->hhdr.error_code , start , end-start);
		}
		else if(!strncasecmp(start,"Host: ",6))
		{
			new_string(&skb->hhdr.host , start , end-start);
		}
		else if(!strncasecmp(start,"Accept-Encoding: ",17))
		{
			new_string(&skb->hhdr.accept_encoding, start , end-start);
		}
		else if(!strncasecmp(start,"Accept: ",8))
		{
			new_string(&skb->hhdr.accept, start , end-start);
		}
		else if(!strncasecmp(start,"User_Agent: ",12))
		{
			new_string(&skb->hhdr.user_agent, start , end-start);
		}
		else if(!strncasecmp(start,"Content-Type: ",14))
		{
			new_string(&skb->hhdr.content_type, start , end-start);
		}
		else if(!strncasecmp(start,"Content_Encoding: ",18))
		{
			new_string(&skb->hhdr.content_encoding, start , end-start);
		}
		else if(!strncasecmp(start,"Content-Length: ",16))
		{
			new_string(&skb->hhdr.content_length, start , end-start);
			skb->hhdr.res_type=HTTP_RESPONSE_TYPE_CONTENTLENGTH;
		}
		else if(!strncasecmp(start,"Transfer-Encoding: chunked",26))
		{
			new_string(&skb->hhdr.transfer_encoding, start , end-start);
			skb->hhdr.res_type=HTTP_RESPONSE_TYPE_CHUNKED;
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
	skb->http_data=start;
	if(!skb->http_data)
	{
		return ERROR;
	}
	
	//////////////include /r/n/r/n ///////
	skb->httph_len = skb->http_data - skb->http_head;
	
	return OK;
}

int decode_tcp(struct _skb *skb)
{
	skb->tcp = (struct tcphdr *)(skb->pload+skb->iph_len);
	if(!skb->tcp)
		return -1;
	skb->tcph_len = 4 * skb->tcp->doff;
	skb->tcp_len = skb->ip_len-skb->iph_len;
	
	if(-1==decode_http(skb))
		return -1;

	return 0;

}

int decode_ip(struct _skb *skb)
{
	
	skb->iph = (struct iphdr *)(skb->pload);
	if(!skb->iph)
		return -1;
	if(skb->iph->ihl < 5 || skb->iph->version != 4)
		return -1;
	skb->ip_len = ntohs(skb->iph->tot_len);
	if(skb->ip_len != skb->pload_len)
		return -1;
	skb->iph_len=4 * skb->iph->ihl;
	if (skb->ip_len < skb->iph_len)
		return -1;
	
	
	if(skb->iph->protocol!=IPPROTO_TCP)
		return -1;
	
	if(-1==decode_tcp(skb))
		return -1;
	
	
	return 0;
}

int decode(struct _skb* skb)
{

	if(-1==decode_ip(skb))
	{
		return -1;
	}
	/*
	char sip[COMM_MAX_LEN],dip[COMM_MAX_LEN];
	ip2addr(sip , ntohl(skb->iph->saddr));
	ip2addr(dip , ntohl(skb->iph->daddr));
	debug_log("%s-->%s seq:%lu , ack_seq:%lu \n" , 
		sip ,dip,
		ntohl(skb->tcp->seq) , ntohl(skb->tcp->ack_seq));
	*/

	return 0;
}

//////////////////////////////////////////////////////////////

static int queue_cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    struct nfqnl_msg_packet_hdr *ph;
	struct _skb skb;

	memset(&skb , '\0' , sizeof(struct _skb));
    //get unique ID of packet in queue
    ph = nfq_get_msg_packet_hdr(nfa);
    if(!ph)
    {
        return -1;
    }
	skb.pkt_id= ntohl(ph->packet_id);

    //get payload
    skb.pload_len = nfq_get_payload(nfa, &(skb.pload));
    if(skb.pload_len <=0)
    {
      	goto send;
    }

	skb.pload[skb.pload_len]='\0';
	if(-1==decode(&skb))
	{
		goto send;
	}

	if(-1==dispath(&skb))
	{
		goto send;
	}
	
	send:
	nfq_set_verdict(qh, skb.pkt_id, NF_ACCEPT, skb.pload_len, skb.pload);
	
    return 0;
    
}

int nfq()
{
	int len, fd;
    char buf[BUFSIZE]={0};
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
	//call nfq_open() to open a NFQUEUE handler
    h = nfq_open();
    if(!h)
    {
        fprintf(stderr, "error during nfq_open()\n");
        exit(1);
    }

    //unbinging existing nf_queue handler for PE_INET(if any)
    if(nfq_unbind_pf(h, PF_INET) < 0)
    {
        fprintf(stderr, "error during nfq_unbind_pf()\n");
        exit(1);
    }

    //binding nfnetlink_queue as nf_queue handler for PF_INET
    if(nfq_bind_pf(h, PF_INET) < 0)
    {
        fprintf(stderr, "error during nfq_bind_pf()\n");
        exit(1);
    }

    //binding this socket to queue '0'
    qh = nfq_create_queue(h, 0, &queue_cb, NULL);
    if(!qh)
    {
        fprintf(stderr,"error during nfq_create_queue()\n");
        exit(1);
    }

    //setting copy_packet mode
    if(nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0)
    {
        fprintf(stderr, "can't set packet_copy_mode\n");
        exit(1);
    }

    //get the file descriptor associated with the nfqueue handler
    fd = nfq_fd(h);

    //handle a packet received from the nfqueue subsystem
    CONTINUE:
    while ((len = recv(fd, buf, BUFSIZE, 0)) && len >= 0)
    {
        nfq_handle_packet(h, buf, len);
    }
	debug_log("error len=%d" ,len);
	sleep(2);
	goto CONTINUE;
    nfq_destroy_queue(qh);
    nfq_close(h);
    return 0;
}
void start_work(struct task_info *ti)
{
    signal_ignore();
	INIT_LIST_HEAD(&httpc_list);
	init_plug();
	init_thpool(2);
	thpool_add_job(timeout , NULL);
	nfq();
}

int main(int argc, const char *argv[])
{
	//capure http protocol packets from kernel
	system("iptables -D INPUT -p tcp --sport 80 -j QUEUE");
	system("iptables -D OUTPUT -p tcp --dport 80 -j QUEUE");
	system("iptables -A INPUT -p tcp --sport 80 -j QUEUE");
	system("iptables -A OUTPUT -p tcp --dport 80 -j QUEUE");
	
	init_mpool(1*1024*1024);//256M
	new_task(1, 1, start_work);
	task_manage();
	return 0;
}

