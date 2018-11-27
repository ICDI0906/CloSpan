#include <iostream>
#include "Global.h"
#include "SeqTree.h"
#include "Global.cpp"
#include <time.h>
#include "ProjDB.h"
#include "ProjDB.cpp"
#include "ClosedSeqTree.h"
#include "ClosedSeqTree.cpp"
#include "ClosedTree.h"
#include "ClosedTree.cpp"
#include "MemMap.h"
#include "LinumMemMap.cpp"
// should add the cpp files
#if defined (_ANOTHER_CLOSED_APPROACH)
TreeNode *root;
TreeNode *currentLevel;
ReverseHashTable *reverseTable; // from NumOfItems to TreeNode*, multimap
int closed_maxFreq[256];
#endif
using namespace std;
void InitApp(const char* filename, const double dSupport, const int nItemCount){

    CreateTimers(3);
    gN_ITEMS = (nItemCount+1); // because it is zero based.
    gpErrFile = file_open(ERRFILE, "w");
    int nFileSize = InitProjDB(filename);

    gpResultFile = gpStatusFile = stdout;

    //Added by Xifeng
#if defined (_ANOTHER_CLOSED_APPROACH)
    root = new TreeNode(-1, 0);
    root -> ItemsetNumber=0;
    root->Parent = NULL;
    currentLevel = root;

    reverseTable = new ReverseHashTable;
    for(int i=0; i< 256; i++)
        closed_maxFreq[i]=0;
#endif
    /////////////////

    fprintf(gpStatusFile, "1LPrefixSpan: A sequential pattern mining algorithm.\n");
    fprintf(gpStatusFile, "Implemented by Behzad Mortazavi-Asl, in IDBL, SFU\n");
    fprintf(gpStatusFile, "All rights reserved!\n");
    fprintf(gpStatusFile, "Data set (%.3f MB): %s\n",
            nFileSize/(1024.0*1024.0), filename);
    fprintf(gpStatusFile, "# of items: %d\n", nItemCount);
    fprintf(gpStatusFile, "Support threshold: %.3f%%\n", dSupport*100);
    fprintf(gpStatusFile, "-----------------------\n");
}

void CloseApp(){

    DeleteTimers();
    freemem ((void**) &gnArrLargeCount);
    freemem ((void**) &buf_idx);
    CloseProjDB();
    fclose (gpErrFile);
}

void PrefixSpan(struct PROJ_DB *pDB){

    int i=0, j=0, nFreqCount=0;

    //add by Xifeng, currentLevel will be modified to next level
#if defined (_ANOTHER_CLOSED_APPROACH)
    if (addSequence(pDB, &currentLevel, reverseTable)==EQUAL_PROJECT_DB_SIZE) {
        clean_projcted_db(pDB, &nFreqCount);
        return;
    }
#endif

    // scan sequence database once, find length-1 sequences
    struct PROJ_DB *proj_db=make_projdb_from_projected_db(pDB, &nFreqCount);


    if (nFreqCount>0)
    {
        for (i=0; i<nFreqCount; i++)
        {

            if( proj_db[i].m_nSup < gSUP )
            {
                n_total_mem-=(proj_db[i].m_nPatLen*sizeof(int));
                freemem ((void**) &(proj_db[i].m_pnPat));
                for (j=0; j<proj_db[i].m_nSup; j++)
                {
                    if (proj_db[i].m_pProjSeq[j].m_nProjCount > 1)
                    {
                        n_total_mem-=(proj_db[i].m_pProjSeq[j].m_nProjCount*sizeof(int*));
                        freemem ((void**) &(proj_db[i].m_pProjSeq[j].m_ppSeq));
                    }
                }
                n_total_mem-=(proj_db[i].m_nMaxSup*sizeof(struct PROJ_SEQ));
                freemem ((void**) &(proj_db[i].m_pProjSeq));

            } else n_proj_db++;
        }
        for (i=0; i<nFreqCount; i++)
        {
            if (proj_db[i].m_nSup >= gSUP)
            {
                PrefixSpan (&(proj_db[i]));
            }
#if defined (_ANOTHER_CLOSED_APPROACH)
            else {
                if (addSequence(&proj_db[i], &currentLevel, reverseTable) != EQUAL_PROJECT_DB_SIZE)
                    currentLevel = currentLevel->Parent;
            }
#endif
        }
        n_total_mem-=(nFreqCount*sizeof(struct PROJ_DB));
        freemem ((void**) &proj_db);
    }
    //add by Xifeng, currentLevel will be modified to its parent level
#if defined (_ANOTHER_CLOSED_APPROACH)
    currentLevel = currentLevel->Parent;
#endif
}


int main(int argc, char ** argv) {
//    if (argc != 4)
//    {
//        gpErrFile = file_open(ERRFILE, "w");
//        if (gpErrFile!=NULL)
//        {
//            fprintf (gpErrFile, "Usage: 1LPrefixSpan <filename> <support> <itemcount>\n");
//            fclose (gpErrFile);
//        }
//        printf ("Usage: 1LPrefixSpan <filename> <support> <itemcount>\n");
//        exit (-1);
//    }
    InitApp("data.in", 1.0, 1);
    ResetTimer(0);
    /////////////////


    int i=0, j=0, nFreqCount=0;
    // scan sequence database once, find length-1 sequences
    struct PROJ_DB *proj_db=make_projdb_from_org_dataset(1.0, &nFreqCount);


    if (nFreqCount>0)
    {
        for (i=0; i<nFreqCount; i++)
        {
            if (proj_db[i].m_nSup < gSUP)
            {
//added by xifeng
#if defined (_ANOTHER_CLOSED_APPROACH)
                if (addSequence(&proj_db[i], &currentLevel, reverseTable) != EQUAL_PROJECT_DB_SIZE)
                    currentLevel = currentLevel->Parent;
#endif
                for (j=0; j<proj_db[i].m_nSup; j++)
                {

                    if (proj_db[i].m_pProjSeq[j].m_nProjCount > 1) {
                        n_total_mem-=(proj_db[i].m_pProjSeq[j].m_nProjCount*sizeof(int*));
                        freemem ((void**) &(proj_db[i].m_pProjSeq[j].m_ppSeq));
                    }
                }
                n_total_mem-=(proj_db[i].m_nMaxSup*sizeof(struct PROJ_SEQ));
                freemem ((void**) &(proj_db[i].m_pProjSeq));
            } else n_proj_db++;
        }
        for (i=0; i<nFreqCount; i++)
        {
            if (proj_db[i].m_nSup >= gSUP)
            {
                PrefixSpan (&(proj_db[i]));
                //fprintf(gpStatusFile, "|");
            }
        }
        n_total_mem-=(nFreqCount*sizeof(struct PROJ_DB));
        freemem ((void**) &proj_db);
    }

    double TimeDiff = GetTimeDiff(0);
    fprintf(gpResultFile, "%.3f seconds (Total running time)\n", TimeDiff );
    for (i=1; gnArrLargeCount[i]>0; i++)
    {
        fprintf(gpResultFile, "Large %d : %d\n", i, gnArrLargeCount[i]);
        gnArrLargeCount[0]+=gnArrLargeCount[i];
    }

    fprintf(gpResultFile, "Total of %d large items.\n", gnArrLargeCount[0]);
    fprintf(gpResultFile, "# of projected datasets: %d\n", n_proj_db);
    fprintf(gpResultFile, "Maximum memory usage: %.3fMB\n",
            double(n_max_mem)/(1024.0*1024.0));

#if defined (_ANOTHER_CLOSED_APPROACH)
    NodeVector::iterator it, endit;
    for (it=root->Children->begin(), endit=root->Children->end(); it != endit; it++) {
        closed_maxPruning((*it), root);
    }
    FILE *closed_maxFile = NULL;
    closed_maxFile = file_open( "ClosedMaxset.txt", "w" );


    fprintf(closed_maxFile, "CLOSED\n");

    for (it=root->Children->begin(), endit=root->Children->end(); it != endit; it++) {
        if ((*it)->Parent == root){}
            //(*it)->Print("(", closed_maxFile);
    }

    fprintf( closed_maxFile, "%.3f seconds (\nTotal running time)\n\n", TimeDiff );

    fprintf(closed_maxFile, "Total # of TreeNode: %d\n", zzz);


    for (i=0; i<256; i++)
        if (closed_maxFreq[i] != 0)
            fprintf(closed_maxFile, "Closed/Max %d : %d\n", i, closed_maxFreq[i]);

    fprintf(closed_maxFile, "\nTotal of %d large items.\n", gnArrLargeCount[0] );
    fprintf(closed_maxFile, "# of projected datasets: %d\n", n_proj_db );
    fprintf(closed_maxFile, "Maximum memory usage: %.6fMB\n", double(n_max_mem)/(1024.0*1024.0) );

    fprintf(closed_maxFile, "%.3f seconds (Total running time after sort, and output.)\n", GetTimeDiff(0) );
    fclose(closed_maxFile );
#endif

    /////////////////
    fprintf( gpResultFile, "%.3f seconds (Total running time after sort, and output.)\n", GetTimeDiff(0) );

    CloseApp();
    return 0;
}