
#ifndef __MSTRING_H__
#define __MSTRING_H__

/*  P R O T O T Y P E S  *************************************************/

int kmpSearch(char *, int , char *, int );
int *make_skip(char *ptrn, int plen);
int *make_shift(char *ptrn, int plen);

int bmSearch(char *, int , char *, int , int *, int *);
int mSearch(char *buf, int blen, char **ptrn, int* plen ,int num,
				int* offset , int* depth , int* distance,int **skip, int **shift);






#endif  /* __MSTRING_H__ */
