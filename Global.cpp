//
// Created by 张开顺 on 2018/11/27.
//

#include <string.h>
#include "Global.h"
#include <errno.h>
//////////////////////////////////////////////////////////////////////
// variables.
//////////////////////////////////////////////////////////////////////
FILE *gpResultFile=NULL;// time taken, # of freq items and # of each large item.
FILE *gpStatusFile=NULL;// on going progress status of program.

FILE *gpErrFile=NULL;		// file handle containing data set to be mined.

int gMAX_PAT_LEN=30;	// maximum length of a frequent sequence.
int gN_ITEMS=10000;		// # of items in the database.
double gSUP=0.0;		// requested support for frequent patterns.
int* gnArrLargeCount=NULL;// # of large patterns of different lengths.
int gnCustCount=0;  // number of customers in database.
int* buf_idx=0;

struct COUNTER* inter = NULL;
struct COUNTER* intra = NULL;
int* inter_freq_idx = NULL;
int* intra_freq_idx = NULL;

struct timeb *pTimer=0;
FILE *file_open(const char *f_name, const char *mode){

    FILE *f=fopen(f_name, mode);

    if (f==0){
        if (gpErrFile != NULL)
            fprintf(gpErrFile , "Fail to open file %s with mode %s\n", f_name, mode);
        else
            printf("Fail to open file %s with mode %s\n", f_name, mode);
        exit(-1);
    }
    return(f);
}

void CreateTimers(int nCount){
    pTimer=(struct timeb*)memalloc(sizeof(struct timeb)*nCount);
}

double GetTimeDiff(int nTimer){

    struct timeb endTime;
    ftime(&endTime);			// record end time.
    return (double(endTime.time - pTimer[nTimer].time) +
            (endTime.millitm - pTimer[nTimer].millitm) / 1000.0);
}

void ReportError(){

    fprintf(gpErrFile, "%s\n", strerror(errno));
}

void DeleteTimers() { freemem ((void**) &pTimer); }