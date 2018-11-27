//
// Created by 张开顺 on 2018/11/27.
//

// LinuxMemMap.cpp: implementation of memory maped file.
//
//////////////////////////////////////////////////////////////////////

#if !defined(WIN32)

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "MemMap.h"
#include "Global.h"

struct mem_map{
    unsigned int m_nCurWnd;
    unsigned int m_nFileSize;
    unsigned int m_nWndSize;

    void* m_pLastAddr;
    void* m_lpMemViewFile;
    int m_hFile;
};

//////////////////////////////////////////////////////////////////////
// Function prototypes

//////////////////////////////////////////////////////////////////////

struct mem_map* CreateMemMap(const char* mem_file, const unsigned int nWndSize){

    struct mem_map* pMem=(struct mem_map*)memalloc(sizeof(struct mem_map));
    int nMode = 420;

    pMem->m_hFile = open(mem_file, O_RDONLY, nMode);
    if (pMem->m_hFile==-1) {
        fprintf(gpErrFile, "Failed to open memory file '%s'.\n", mem_file);
        ReportError();
        freemem ((void**) &pMem);
        exit(1);
    }

    pMem->m_nCurWnd = 0;
    pMem->m_nFileSize = lseek (pMem->m_hFile, 0, SEEK_END);
    lseek (pMem->m_hFile, 0, SEEK_SET);
    if (pMem->m_nFileSize <= nWndSize) pMem->m_nWndSize = pMem->m_nFileSize;
    else pMem->m_nWndSize = nWndSize;

    pMem->m_lpMemViewFile = mmap(0, pMem->m_nWndSize,
                                 PROT_READ, MAP_SHARED,
                                 pMem->m_hFile, 0);
    if (pMem->m_lpMemViewFile == MAP_FAILED){
        fprintf(gpErrFile, "Failed to create view of memory map-file.\n");
        ReportError();
        close (pMem->m_hFile);
        freemem ((void**) &pMem);
        exit(1);
    }

    if (pMem->m_nFileSize <= nWndSize){
        pMem->m_pLastAddr = ((char*)(pMem->m_lpMemViewFile) + pMem->m_nFileSize);
    } else {
        int *dataset = (int*) ((char*)(pMem->m_lpMemViewFile) + pMem->m_nWndSize);
        for (; *dataset!=-2; dataset--);
        pMem->m_pLastAddr = (void*)(dataset+1);
    }

    return pMem;
}

void CloseMemMap(struct mem_map** ppMem){

    if (ppMem!=NULL && *ppMem!=NULL) {
        if ((*ppMem)->m_lpMemViewFile != NULL)
            munmap ((caddr_t)(*ppMem)->m_lpMemViewFile, (*ppMem)->m_nWndSize);
        close ((*ppMem)->m_hFile);
    }
    freemem ((void**) ppMem);
}

int GetCurWnd(const struct mem_map* pMem){ return pMem->m_nCurWnd; }
void* GetStartOfMap(const struct mem_map* pMem){ return pMem->m_lpMemViewFile; }
void* GetLastAddrOfMap(const struct mem_map* pMem){ return pMem->m_pLastAddr; }
int GetMemMapFileSize(const struct mem_map* pMem){ return pMem->m_nFileSize; }

#endif // not WIN32
//////////////////////////////////////////////////////////////////////
// END
