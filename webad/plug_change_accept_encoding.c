
#include "main.h"

PRIVATE int change_accept_encoding(void* data)
{	
	struct http_conntrack* httpc = (struct http_conntrack *)data;
	struct _skb *skb=httpc->skb;
	
	char* tmp;
	tmp=strstr(skb->http_head, "Accept-Encoding: gzip");
	if(!tmp)
	{
		return -1;
	}
	
	tmp[0]='B';

	skb->tcp->check=tcp_chsum(skb->iph , skb->tcp , skb->tcp_len);
	return 0;
}

int init_change_accept_encoding()
{
	//new_check_plug(change_accept_encoding , CHECK_PLUG_PRE);
	return OK;
}
