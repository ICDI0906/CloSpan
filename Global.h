//
// Created by 张开顺 on 2018/11/27.
//
#include "SeqTree.h"
#ifndef CLOSPAN_STUDY_GLOBAL_H
#define CLOSPAN_STUDY_GLOBAL_H

#include "SeqTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/timeb.h>
#define DISK_BASED 1
// 声明一些全局变量，为的是在别的地方使用它，要是赋初值了就只能是定义了
#define FREQUENT "frequent.dat"
#define ERRFILE "error.tmp"

extern FILE *gpResultFile;
extern FILE *gpStatusFile;

extern FILE *gpErrFile;

extern int gMAX_PAT_LEN;
extern int gN_ITEMS;
extern double gSUP;
extern int* gnArrLargeCount;

extern int gnCustCount;
extern int* buf_idx;

// 函数定义
FILE *file_open(const char *f_name, const char *mode);
// inline 函数在函数调用时展开，内敛函数的定义放在头文件中
// 在类中定义默认是内敛函数，而要是放在类外，则需要显示地添加inline来表示内敛函数
// inline 关键词要与内敛函数的定义放在一起
inline void* memalloc(size_t nSize){
    // ok
    void *mem=0;
    if (nSize>0) {

        mem=malloc(nSize); // 返回的就是void * 的指针
        if (mem==0){
//            fprintf(gpErrFile, "Fail to allocate memory with size %zu\n", nSize); // size_
            printf("Fail to allocate memory\n");
            exit(-1);
        }
    }
    return(mem);
}
inline void freemem(void** p){
    //ok
    if (p!=0 && *p!=0){
        free (*p);
        *p=NULL;
    }
}

void ReportError();
void CreateTimers(int nCount);
void DeleteTimers();
extern struct timeb *pTimer;
#define ResetTimer(nTimer) ftime(pTimer+nTimer);

double GetTimeDiff(int nTimer);

#endif //CLOSPAN_STUDY_GLOBAL_H
