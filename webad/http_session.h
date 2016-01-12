#ifndef __HTTP_SESSION_H__
#define __HTTP_SESSION_H__

#include "list.h"
#include "tcp_stream.h"
#include "mstring.h"

typedef enum 
{
	HTTP_TYPE_OTHER,
	HTTP_TYPE_REQUEST_POST,
	HTTP_TYPE_REQUEST_GET,
	HTTP_TYPE_RESPONSE
	
}HTTP_TYPE;

typedef enum 
{
	HTTP_RESPONSE_TYPE_OTHER,
	HTTP_RESPONSE_TYPE_CHUNKED,
	HTTP_RESPONSE_TYPE_CONTENTLENGTH
	
}HTTP_RESPONSE_TYPE;

struct http_hdr
{
	HTTP_TYPE http_type;
	HTTP_RESPONSE_TYPE res_type;
	int httph_len;
	//////GET/////////////
	string uri;
	string host;
	string user_agent;
	string accept_encoding;//gzip,deflate
	string accept;//text/html
	////RESPONSE/////////
	string error_code; //200 404 
	string content_encoding;//gzip,deflate,compress
	string content_type;//text/html
	string content_length;
	string transfer_encoding;	//chunked
};

struct http_request
{
	struct list_head list;
	struct tcp_stream tcps;
	struct http_hdr hhdr;
	int response_num;
	unsigned int js_len;
	struct skb_buf *curr_skb;
};

char* get_data_from_skb(struct skb_buf* skb);
int get_data_len_from_skb(struct skb_buf* skb);

void http_chsum(struct skb_buf* skb);
void change_ip_len(struct skb_buf* skb , unsigned long last_ip_len);
void change_seq(struct skb_buf* skb , unsigned long last_seq);

void process_http(struct skb_buf *skb ,void (*callback)(void*));
void init_http_session();

#endif

