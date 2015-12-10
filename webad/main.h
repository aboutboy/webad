#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <netdb.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <net/if.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/netfilter.h>
#include "libnetfilter_queue.h"


#include "list.h"
#include "mpool.h"
#include "msocket.h"
#include "mstring.h"
#include "acsmx.h"
#include "regexp.h"
#include "task.h"
#include "plug.h"
#include "thpool.h"
#include "util.h"

#define PRIVATE static 
#define OK		0
#define ERROR   -1

#define BUFSIZE 2048
#define PKT_LEN 1500
#define COMM_MAX_LEN 256
#define MAX_PATTERN_NUM 20

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

struct _skb
{
	struct list_head list;
	int pkt_id;
	int pload_len;
	unsigned char* pload;
	struct iphdr *iph;
	struct tcphdr *tcp;	
	char* http_head;
	char* http_data;
	struct http_hdr hhdr;
	int iph_len;
	int ip_len;
	int tcph_len;
	int tcp_len;
	int httph_len;
	int http_len;
};

struct http_conntrack
{
	struct list_head list;
	unsigned long sip,dip,seq,ack_seq;
	int http_len;
	char host[COMM_MAX_LEN];
	long last_time;
	char insert_js_tag;
	int insert_js_len;
	int insert_js_seq;
	struct _skb* skb;
};


#endif

