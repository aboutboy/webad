#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ACSMX_H
#define ACSMX_H


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
*   Prototypes
*/


#define ALPHABET_SIZE    256     

#define ACSM_FAIL_STATE   -1     


typedef struct _acsm_pattern {      

    struct  _acsm_pattern *next;
    unsigned char         *patrn;
    unsigned char         *casepatrn;
    int      n;
    int      nocase;
    int      offset;
    int      depth;
    unsigned id;
    int      iid;
    

} ACSM_PATTERN;


typedef struct  {    

    /* Next state - based on input character */
    int      NextState[ ALPHABET_SIZE ];  

    /* Failure state - used while building NFA & DFA  */
    int      FailState;   

    /* List of patterns that end here, if any */
    ACSM_PATTERN *MatchList;   

}ACSM_STATETABLE; 


/*
* State machine Struct
*/
typedef struct {
  
	int acsmMaxStates;  
	int acsmNumStates;  

	ACSM_PATTERN    * acsmPatterns;
	ACSM_STATETABLE * acsmStateTable;

        int   bcSize;
        short bcShift[256];

}ACSM_STRUCT;

/*
*   Prototypes
*/
ACSM_STRUCT * acsmNew ();

int acsmAddPattern( ACSM_STRUCT * p, unsigned char * pat, int n,
          int nocase, int offset, int depth, unsigned id, int iid );

int acsmCompile ( ACSM_STRUCT * acsm );

int acsmSearch ( ACSM_STRUCT * acsm,unsigned char * T, int n, 
		  int (*Match)( unsigned id, int index, void * data ),
                  void * data );

void acsmFree ( ACSM_STRUCT * acsm );


#endif
