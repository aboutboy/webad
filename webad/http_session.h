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
	struct list_head http_response_head;
	struct tcp_stream tcps;
	struct http_hdr hhdr;
	struct skb_buf skb_response_cache;
};

char* get_data_from_skb(struct skb_buf* skb);
int get_data_len_from_skb(struct skb_buf* skb);
void process_http(struct skb_buf *skb ,void (*callback)(void*));
void init_http_session();

#endif

