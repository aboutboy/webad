#include "main.h"

#define REDIRECT_HEADER "HTTP/1.1 302 Moved Temporarily\r\n \
    Location: http://www.ifeng.com\r\n  \
    Content-Type: text/html; charset=iso-8859-1\r\n  \
    Content-length: 252\r\n  \
    \r\n \
    <!DOCTYPE HTML PUBLIC /\"-//IETF//DTD HTML 2.0//EN/\">\r\n  \
    <html><head>\r\n  \
    <title>301 Moved Permanently</title>\r\n  \
    </head><body>\r\n  \
    <h1>Moved Permanently</h1>\r\n  \
    <p>The document has moved <a href=\"/\" mce_href=\"/\"\"http://www.moppo.cn\">here</a>.</p>\r\n  \
    </body></html>\r\n\0"

#define REDIRECT_HEADER_LEN strlen(REDIRECT_HEADER)

PRIVATE int redirect_url(void *data)
{
	struct _skb *skb=(struct _skb *)data;
    if(skb->hhdr.http_type != HTTP_TYPE_RESPONSE)
    {
		return ERROR;
	}
	debug_log("redirect url  start ......%s" , skb->http_head);
	memcpy(skb->http_head , REDIRECT_HEADER , REDIRECT_HEADER_LEN+1);
	skb->iph->tot_len=htons(REDIRECT_HEADER_LEN);
	skb->iph->check=ip_chsum(skb->iph);

	skb->tcp->check=tcp_chsum(skb->iph , skb->tcp , skb->tcph_len+REDIRECT_HEADER_LEN);
	skb->pload_len=skb->iph_len+skb->tcph_len+REDIRECT_HEADER_LEN;
	debug_log("redirect url end ......%s" , skb->pload+skb->iph_len+skb->tcph_len);

    return OK;
}

int init_redirect_url()
{
	new_check_plug(redirect_url , CHECK_PLUG_POST);
	return 0;
}

