  
#include "main.h"

#define kmp_islower(c)      ('a' <= (c) && (c) <= 'z')
#define kmp_toupper(c)      ((c) == '\0' || isdigit(c) ? (c):(kmp_islower(c) ? ((c) - 'a' + 'A') : (c)))


/****************************************************************
 *  KMP
 *  Function: mSearch(char *, int, char *, int)
 *
 *  Purpose: Determines if a string contains a (non-regex)
 *           substring.
 *
 *  Parameters:
 *      buf => data buffer we want to find the data in
 *      blen => data buffer length
 *      ptrn => pattern to find
 *      plen => length of the data in the pattern buffer
 *
 *  Returns:
 *      Integer value, 1 on success (str constains substr), 0 on
 *      failure (substr not in str)
 *
 ****************************************************************/

PRIVATE int* computeNext(const char* pattern , const int len)
{
    int i, nextTemp;
    int *next;
    if(!pattern)
    {
        return NULL;
    }
    if(len == 0)
    {
       return NULL;
    }
    next = (int*)new_page(sizeof(int) * len);
    if(!next)
    {
        return NULL;
    }
    next[0] = -1;
    for(i = 1; i < len; i++)
    {
        nextTemp = next[i - 1];
        while(pattern[i - 1] != pattern[nextTemp] && nextTemp >= 0)
        {
            nextTemp = next[nextTemp];
        }
        next[i] = nextTemp + 1;
    }
    return next;
}

int kmpSearch(char *buf, int blen, char *ptrn, int plen)
{
    int i = 0, j = 0, index;
    int *next;
    if(!buf || !ptrn || blen > 1024 || blen<=0||plen>=1024 || plen<=0 )
    {
        return -1;
    }
    next = computeNext(ptrn , plen);
    if(!next)
    {
       return -1;
    }
	
    while(i < blen && j < plen && j >= -1)
    {
       if( j == -1 || buf[i] == ptrn[j]|| kmp_toupper(buf[i]) == kmp_toupper(ptrn[j]))
       {
          i++;
          j++;
       }
       else
       {
          j = next[j];
       }
    }
    if(j == plen)
    {
        index = i - plen;
    }
    else
    {
        index = -1;
    }
    free_page(next);
    return index;
}



/****************************************************************
 *
 *  Function: make_skip(char *, int)
 *
 *  Purpose: Create a Boyer-Moore skip table for a given pattern
 *
 *  Parameters:
 *      ptrn => pattern
 *      plen => length of the data in the pattern buffer
 *
 *  Returns:
 *      int * - the skip table
 *
 ****************************************************************/
int *make_skip(char *ptrn, int plen)
{
    int *skip = (int *) new_page(256 * sizeof(int));
    int *sptr = &skip[256];

    if (skip == NULL)
    {
    	debug_log("new_page error");
		return NULL;
    }

    while(sptr-- != skip)
        *sptr = plen + 1;

    while(plen != 0)
        skip[(unsigned char) *ptrn++] = plen--;

    return skip;
}



/****************************************************************
 *
 *  Function: make_shift(char *, int)
 *
 *  Purpose: Create a Boyer-Moore shift table for a given pattern
 *
 *  Parameters:
 *      ptrn => pattern
 *      plen => length of the data in the pattern buffer
 *
 *  Returns:
 *      int * - the shift table
 *
 ****************************************************************/
int *make_shift(char *ptrn, int plen)
{
    int *shift = (int *) new_page(plen * sizeof(int));
    int *sptr = shift + plen - 1;
    char *pptr = ptrn + plen - 1;
    char c;

    if (shift == NULL)
    {
    	debug_log("new_page error");
		return NULL;
    }

     c = ptrn[plen - 1];

    *sptr = 1;

    while(sptr-- != shift)
    {
        char *p1 = ptrn + plen - 2, *p2, *p3;

        do
        {
            while(p1 >= ptrn && *p1-- != c);

            p2 = ptrn + plen - 2;
            p3 = p1;

            while(p3 >= ptrn && *p3-- == *p2-- && p2 >= pptr);
        }
        while(p3 >= ptrn && p2 >= pptr);

        *sptr = shift + plen - sptr + p2 - p3;

        pptr--;
    }

    return shift;
}



/****************************************************************
 *
 *  Function: mSearch(char *, int, char *, int)
 *
 *  Purpose: Determines if a string contains a (non-regex)
 *           substring.
 *
 *  Parameters:
 *      buf => data buffer we want to find the data in
 *      blen => data buffer length
 *      ptrn => pattern to find
 *      plen => length of the data in the pattern buffer
 *      skip => the B-M skip array
 *      shift => the B-M shift array
 *
 *  Returns:
 *      Integer value, 1 on success (str constains substr), 0 on
 *      failure (substr not in str)
 *
 ****************************************************************/
int bmSearch(char *buf, int blen, char *ptrn, int plen,int *skip, int *shift)
{
	int b_idx = plen;

	if(plen == 0)
	{
		return -1;
	}

	if(!shift||!skip)
	{
		return -1;
	}
    while(b_idx <= blen)
    {
        int p_idx = plen, skip_stride, shift_stride;

        while(buf[--b_idx] == ptrn[--p_idx])
        {

            if(b_idx < 0)
            {
				return -1;
            }

            if(p_idx == 0)
            {
                return b_idx;
            }
        }

        skip_stride = skip[(unsigned char) buf[b_idx]];
        shift_stride = shift[p_idx];

        b_idx += (skip_stride > shift_stride) ? skip_stride : shift_stride;
		
    }

    return -1;
}


int mSearch(char *buf, int blen, char **ptrn, int* plen ,int num,
				int* offset , int* depth , int* distance,int **skip, int **shift)
{

   	int sidx=0;
	int len=blen;
	int found=-1;
	int i;
	
	if(blen>PKT_LEN||blen<=0)
		goto exit;

	for(i=0;i<num; i++)
	{
		if(offset[i]<0||offset[i] > PKT_LEN||
			depth[i]<0||depth[i] > PKT_LEN||
			distance[i]<0||distance[i] > PKT_LEN)
		{
			found=-1;
			goto exit;
		}
		if(!sidx&&offset[i])
		{
			sidx+=offset[i];
			if(sidx>=blen)
			{
				found=-1;
				goto exit;
			}
		}
		if(depth[i])
		{
			len=sidx+depth[i];
			if(len>blen)
			{
				found=-1;
				goto exit;
			}
		}
		
		found=bmSearch(buf+sidx, len-sidx, ptrn[i], plen[i],skip[i],shift[i]);
		
		if(-1!=found)
		{
			if(!distance[i])
			{
				sidx=0;
			}
			else
			{
				sidx+=found+distance[i]+plen[i];
				if(sidx>blen)
				{
					found=-1;
					goto exit;
				}
			}
			len=blen;
			
			continue;
		}
		else
		{
			goto exit;
		}
		
	}
	
	exit:
	return found;
}
void test_kmp()
{
    char* src = "abacabcacabcabcabcattcbab";
    char* target = "abcabcabc";
    debug_log( "\nsrc: %s\ntarget: %s\nThe index is: %d\tBased on KMP algorithm\n", src, target, kmpSearch(src, strlen(src), target,strlen(target)));
}

void test_bm()
{
   /*  char test[] = "\0\0\0\0\0\0\0\0\0CKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\0\0";
    char find[] = "CKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\0\0";

  char test[] = "\x90\x90\x90\x90\x90\x90\xe8\xc0\xff\xff\xff/bin/sh\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90";
     char find[] = "\xe8\xc0\xff\xff\xff/bin/sh"; 
    debug_log("%d\n", bmSearch(test, sizeof(test) - 1, find,
				    sizeof(find) - 1)); */


}

void new_string(string* s , char* c , int l)
{
	s->c=c;
	s->l=l;
}

string* new_mem_string(char*c , int l)
{
	string* s;
	s=(string*)malloc(sizeof(string));
	if(!s)
		return NULL;
	
	s->c=(char*)malloc(l+1);
	if(!s->c)
		return NULL;
	
	memcpy(s->c , c , l);
	s->l=l;
	return s;
}

void free_mem_string(string* s)
{
	if(s->c)
		free(s->c);
	if(s)
		free(s);
}
