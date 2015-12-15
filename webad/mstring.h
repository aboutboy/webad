
#ifndef __MSTRING_H__
#define __MSTRING_H__

typedef struct  _string
{
	char* c;
	int l;
	
}string;

int kmpSearch(char *, int , char *, int );
int *make_skip(char *ptrn, int plen);
int *make_shift(char *ptrn, int plen);

int bmSearch(char *, int , char *, int , int *, int *);
int mSearch(char *buf, int blen, char **ptrn, int* plen ,int num,
				int* offset , int* depth , int* distance,int **skip, int **shift);



void new_string(string* s , char* c , int l);
string* new_mem_string(char*c , int l);
void free_mem_string(string* s);


#endif  /* __MSTRING_H__ */
