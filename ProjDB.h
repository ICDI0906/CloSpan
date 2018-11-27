//
// Created by 张开顺 on 2018/11/27.
//
#include "SeqTree.h"
#ifndef CLOSPAN_STUDY_PROJDB_H
#define CLOSPAN_STUDY_PROJDB_H
//////////////////////////////////////////////////////////////////////
// exported structures and variables.
//////////////////////////////////////////////////////////////////////
struct COUNTER{
    int s_id;	    // last sequence updated this counter.
    int count;	  // accumulated count.
};
struct PROJ_SEQ{
    int m_nProjCount;  // number of projected sequences in m_ppSeq.
    int **m_ppSeq;     // pointer to first element if each projection.
};

struct PROJ_DB{
    int  m_nPatLen;    // pattern length of projected database.
    int *m_pnPat;      // pattern of projected database.

    int  m_nMaxSup;    // maximum support that this projection can have.
    int  m_nSup;       // actual support found in dataset.
    int  m_nVer;       // last sequence id that supported this pattern.
    int  m_nSeqSize;   // size of projected sequnce in m_pProjSeq[0].
    struct PROJ_SEQ *m_pProjSeq;  // projected sequences.
#if defined( _USE_OUTPUT_BUFFER )
    Sequence * OutputBuf; // Added by Ramin.
#endif
#if defined( _CALC_I_NUM_OF_ITEMS )
    bool ItemIsIntra;
    int  Item;
    long NumOfItems; // Added by Ramin.
#endif
};

extern int n_proj_db;  // number of projected sequences processed.
extern int n_max_mem; // maximum memory usage by projected databases.
extern int n_total_mem; // total memory usage by projected databases.

//////////////////////////////////////////////////////////////////////
// function definition.
//////////////////////////////////////////////////////////////////////
int InitProjDB(const char* filename);
void CloseProjDB();
struct PROJ_DB* make_projdb_from_org_dataset(const double dSupport,
                                             int* pnFreqCount);
struct PROJ_DB* make_projdb_from_projected_db(const struct PROJ_DB* pDB,
                                              int* pnFreqCount);

struct PROJ_DB* my_make_projdb(const struct PROJ_DB* pDB,
                               int* pnFreqCount);
//added by xifeng, clean_projected_db is used to release some space (in the
//last part of make_projdb_from_projected_db)
//the design of make_projdb_from_projected_db release the pDB-> space in the
//function , while pDB itself releasing is done in Prefix-Span function.
#if defined (_ANOTHER_CLOSED_APPROACH)
void clean_projcted_db(const struct PROJ_DB* pDB, int* pnFreqCount);
#endif

//////////////////////////////////////////////////////////////////////
// Function Prototype
//////////////////////////////////////////////////////////////////////
void PrintProjDBs(const struct PROJ_DB *proj_db, const int nCount);

bool OutputPattern( const int *const pat, const int nPatLen, const int nSup );

#endif //CLOSPAN_STUDY_PROJDB_H
