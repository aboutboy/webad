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

struct http_conntrack
{
	struct list_head list; 
	long last_time;
	struct tuple4 addr;
	struct http_hdr httph;
	struct skb_buf skb;
};

struct http_conntrack_get
{
	struct list_head list; 
	struct http_conntrack httpc;
	struct list_head http_conntrack_response;
};

struct http_contrack_response
{
	struct list_head list;
	struct http_conntrack httpc;
};

#endif

