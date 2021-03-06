#include "main.h"

struct nfq_q_handle *gqh;

void http_session_handle(void * data)
{
	struct http_request* httpr=(struct http_request*)data;
	struct skb_buf* skb=httpr->curr_skb;
	
	switch(skb->result)
	{
		case RESULT_FROM_CLIENT:
			//debug_log("html get------> %s",skb->pload+(skb->pload_len-skb->data_len));
			plug_hook(data , PLUG_EXTERN_TYPE_REQUEST);
			return;
		case RESULT_FROM_SERVER:
			//printf("111111len %d html ------> %s" ,get_data_len_from_skb(skb),get_data_from_skb(skb));
			plug_hook(data , PLUG_EXTERN_TYPE_RESPONSE);
			//printf("111111len %d html ------> %s" ,get_data_len_from_skb(skb),get_data_from_skb(skb));
			return;
		case RESULT_IGNORE:
		case RESULT_CACHE:
		default:
			break;
	}

	nfq_set_verdict(gqh, skb->packet_id, NF_ACCEPT, skb->pload_len, skb->pload);
}

void tcp_stream_handle(void * data)
{
	struct skb_buf* skb=(struct skb_buf*)data;
	switch(skb->result)
	{
		case RESULT_FROM_CLIENT:
		case RESULT_FROM_SERVER:
			//debug_log("!!!!!!seq %lu len %d",skb->seq ,skb->data_len);
			process_http(skb , http_session_handle);
			break;
		case RESULT_IGNORE:
			//debug_log("~~~~seq %lu len %d",skb->seq ,skb->data_len);
			nfq_set_verdict(gqh, skb->packet_id, NF_ACCEPT, skb->pload_len, skb->pload);
			break;
		case RESULT_CACHE:
		default:
			break;
	}
	

}

static int queue_cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    struct nfqnl_msg_packet_hdr *ph;
	struct skb_buf skb;

    //get unique ID of packet in queue
    ph = nfq_get_msg_packet_hdr(nfa);
    if(!ph)
    {
        return -1;
    }
        skb.packet_id= ntohl(ph->packet_id);

    //get payload
    skb.pload_len = nfq_get_payload(nfa, &(skb.pload));
    if(skb.pload_len <=0)
    {
    	nfq_set_verdict(qh, skb.packet_id, NF_ACCEPT, skb.pload_len, skb.pload);
        return -1;
    }

    skb.pload[skb.pload_len]='\0';
	
	process_tcp(&skb ,tcp_stream_handle);
	
    return 0;
    
}

int nfq()
{
	int len, fd;
    char buf[BUFSIZE]={0};
    struct nfq_handle *h;
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
    gqh = nfq_create_queue(h, 0, &queue_cb, NULL);
    if(!gqh)
    {
        fprintf(stderr,"error during nfq_create_queue()\n");
        exit(1);
    }

    //setting copy_packet mode
    if(nfq_set_mode(gqh, NFQNL_COPY_PACKET, 0xffff) < 0)
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
    nfq_destroy_queue(gqh);
    nfq_close(h);
    return 0;
}
void start_work(struct task_info *ti)
{
    signal_ignore();
	init_plug();
	init_plug_extern();
	init_tcp_stream();
	init_http_session();
	nfq();
}

int main(int argc, const char *argv[])
{
	//capure http protocol packets from kernel
	
	//new_task(1, 1, start_work);
	//task_manage();

	start_work(NULL);
	return 0;
}

