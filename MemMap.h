//
// Created by 张开顺 on 2018/11/27.
//

#ifndef CLOSPAN_STUDY_MEMMAP_H
#define CLOSPAN_STUDY_MEMMAP_H

struct mem_map;

struct mem_map* CreateMemMap(const char* mem_file, const unsigned int nWndSize);
void CloseMemMap(struct mem_map** ppMem);
int GetCurWnd(const struct mem_map* pMem);
void* GetStartOfMap(const struct mem_map* pMem);
void* GetLastAddrOfMap(const struct mem_map* pMem);
int GetMemMapFileSize(const struct mem_map* pMem);

#endif //CLOSPAN_STUDY_MEMMAP_H
