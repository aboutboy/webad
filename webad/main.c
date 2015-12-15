#include "main.h"

struct nfq_q_handle *gqh;

void tcp_stream(void * data)
{
	struct skb_buf* skb=(struct skb_buf*)data;
	switch(skb->result)
	{
		case RESULT_HANDLE:
			debug_log("seq %lu len %d",skb->seq ,skb->data_len);
			nfq_set_verdict(gqh, skb->packet_id, NF_ACCEPT, skb->pload_len, skb->pload);
		case RESULT_IGNORE:
			nfq_set_verdict(gqh, skb->packet_id, NF_ACCEPT, skb->pload_len, skb->pload);
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
	
	process_tcp(&skb ,tcp_stream);
	
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
	init_tcp_stream();
	nfq();
}

int main(int argc, const char *argv[])
{
	//capure http protocol packets from kernel
	system("iptables -D INPUT -p tcp --sport 80 -j QUEUE");
	system("iptables -D OUTPUT -p tcp --dport 80 -j QUEUE");
	system("iptables -A INPUT -p tcp --sport 80 -j QUEUE");
	system("iptables -A OUTPUT -p tcp --dport 80 -j QUEUE");
	
	new_task(1, 1, start_work);
	task_manage();
	return 0;
}

