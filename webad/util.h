#ifndef __UTIL_H__
#define __UTIL_H__

#include <linux/ip.h>
#include <linux/tcp.h>

#define PRIVATE static 
#define OK		0
#define ERROR   -1

#define BUFSIZE 2048
#define PKT_LEN 1500
#define COMM_MAX_LEN 256

#define MAX_DEBUG_FILE_SIZE     60*1024*1024  //60M
#define LOG_FILE_NAME "./webad.log"

char **mSplit(char *, char *, int, int *, char);
void mSplitFree(char ***pbuf, int num_toks);
int mContainsSubstr(char *, int, char *, int);
char *copy_argv(char **argv);
int strip(char *data);
void ts_print(long sec, char *timebuf);
void ts_print1(long sec, char *timebuf);
void mac2str(char* dmac , unsigned char* smac);
void ip2addr(char* addr , unsigned long ip);
int digital(const char* s);
int del_end_enter(char* src , int len);
void debug_log(const char *fmt, ...);
int get_client_mac(char* eth ,char* mac);
float get_pcpu();
float get_pmem();
float get_pdev();
int proto2str(char proto , char* str);
int get_all_ifdev(char* re_str ,char* split_str);
int get_operationsys_name(char* name);
int get_kernel_version(char* version);
int get_computer_name(char* name);
int get_machine(char* name);
int get_total_mem(char* mem);
long long htonll(long long s);
long long ntohll(long long s);
int how_many_digits(long s);
int mnanosleep(long nasec);
void data_time_format(char* dtime , char* stime);
long get_current_sec();

unsigned short in_cksum(unsigned short *addr, int len);
unsigned short ip_chsum(struct iphdr *iph);
unsigned short tcp_chsum(struct iphdr *iph , struct tcphdr *tcp ,int tcp_len);
void hex2i(const char* hex_str, int* des_i);
void i2hex(const int src_i , char* hex_str);

void *test_malloc(int x);

void test_free(void* x);

#endif
