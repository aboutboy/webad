
#include "main.h"

PRIVATE int change_seq(void* data)
{	
	struct http_conntrack* httpc = (struct http_conntrack *)data;

	//after insert_js
	if(httpc->insert_js_tag==ERROR || !httpc->insert_js_seq)
    {
	   return ERROR;
	}
	if(ntohl(httpc->insert_js_seq)>=ntohl(httpc->skb->tcp->seq))
	{
		return ERROR;
	}
	//debug_log("````````````%d---------%s\n````````````````````\n" ,httpc->skb->http_len, httpc->skb->http_head);
	httpc->skb->tcp->seq=htonl(ntohl(httpc->skb->tcp->seq)+httpc->insert_js_len);
	httpc->skb->tcp->check=tcp_chsum(httpc->skb->iph , httpc->skb->tcp , httpc->skb->tcp_len);
	return OK;
}

int init_change_seq()
{
	new_plug(change_seq , PLUG_TYPE_OTHER);
	return OK;
}
