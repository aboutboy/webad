
#include "main.h"

PRIVATE int change_accept_encoding(void* data)
{	
	struct http_conntrack* httpc = (struct http_conntrack *)data;
	struct _skb *skb=httpc->skb;

	if(!strncasecmp(skb->hhdr.accept_encoding.c, "Accept-Encoding: gzip" ,21))
	{
		skb->hhdr.accept_encoding.c[0]='B';
		skb->tcp->check=tcp_chsum(skb->iph , skb->tcp , skb->tcp_len);
	}

	return OK;
}

int init_change_accept_encoding()
{
	new_plug(change_accept_encoding , PLUG_TYPE_GET);
	return OK;
}
