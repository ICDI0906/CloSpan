//
// Created by 张开顺 on 2018/11/27.
//

// ProjDB.cpp: implementation of projected database structure.
//
//////////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>

#include "ProjDB.h"
#include "Global.h"
#include "MemMap.h"

//////////////////////////////////////////////////////////////////////
// variables.
//////////////////////////////////////////////////////////////////////
int n_proj_db =0;   // number of projected sequences processed.
int n_max_mem=0;		// maximum memory usage by projected databases.
int n_total_mem=0;	// total memory usage by projected databases.
//struct COUNTER* inter=NULL;
//struct COUNTER* intra=NULL;
//int* inter_freq_idx=NULL;
//int* intra_freq_idx=NULL;
struct mem_map	*pDatasetMemMap=NULL;
//////////////////////////////////////////////////////////////////////
// functions.
//////////////////////////////////////////////////////////////////////
int InitProjDB(const char* filename){

    inter=(struct COUNTER*)memalloc(sizeof(struct COUNTER)*gN_ITEMS);
    intra=(struct COUNTER*)memalloc(sizeof(struct COUNTER)*gN_ITEMS);
    inter_freq_idx=(int*)memalloc(sizeof(int)*gN_ITEMS);
    intra_freq_idx=(int*)memalloc(sizeof(int)*gN_ITEMS);

    pDatasetMemMap = CreateMemMap(filename, 1536*1024*1024);
    return GetMemMapFileSize (pDatasetMemMap);
}
void CloseProjDB(){

    freemem ((void**) &inter);
    freemem ((void**) &intra);
    freemem ((void**) &inter_freq_idx);
    freemem ((void**) &intra_freq_idx);
    CloseMemMap (&pDatasetMemMap);
}

struct PROJ_DB* make_projdb_from_org_dataset(const double dSupport,
                                             int* pnFreqCount)
{

    int i=0, j=0, nSize=0, nCount=0, nPatLen=0, nMaxSeqLen=0, nSeqLen=0;
    int *d=0, *dataset = (int*) GetStartOfMap (pDatasetMemMap);
    int *lastAddr = (int*) GetLastAddrOfMap (pDatasetMemMap);
    struct PROJ_DB* proj_db=NULL;
    struct PROJ_DB* tempDB=NULL;
    struct PROJ_SEQ* tempSeq=NULL;
    struct COUNTER* pCnter=0;

    *pnFreqCount = 0;

    fprintf(gpStatusFile, "\tCounting 1-Item sets ... \n"); fflush(gpStatusFile);
    ResetTimer(1);

    memset( inter_freq_idx, 0, sizeof(int)*gN_ITEMS );
    memset( inter, 0, sizeof(struct COUNTER)*gN_ITEMS );

    // Scaning DB to count the frequent items.
    for( nCount=0; dataset<lastAddr; dataset++ )
    {
        nCount++; nPatLen=nSeqLen=nPatLen=0;
        for (; *dataset!=-2; dataset++)
        {
            for (; *dataset!=-1; dataset++)
            {
                // eat up consecutive identical numbers.
                while (dataset[0]==dataset[1]) dataset++;
                nPatLen++; nSeqLen++;
                pCnter=inter + (*dataset);

                if (pCnter->s_id<nCount)
                {
                    pCnter->s_id=nCount;
                    pCnter->count++;
                    if (dataset[2]!=-2) inter_freq_idx[*dataset]++;
                }
                nSeqLen++;
            }
            nSeqLen++;
        }
        if (gMAX_PAT_LEN < nPatLen) gMAX_PAT_LEN = nPatLen;
        if (nMaxSeqLen < nSeqLen) nMaxSeqLen=nSeqLen;
    }

    gnCustCount = nCount;
    gSUP=(nCount*dSupport);
    gMAX_PAT_LEN+=2; // since we don't use the first and last index.
    gnArrLargeCount=(int*)memalloc(sizeof(int)*gMAX_PAT_LEN);
    memset(gnArrLargeCount, 0, sizeof(int)*gMAX_PAT_LEN);
#if defined( _FIND_CLOSED_SEQS )
    gnResSizeCount = (int*) memalloc( sizeof(int) * gMAX_PAT_LEN );
	memset( gnResSizeCount, 0, sizeof(int) * gMAX_PAT_LEN );
#endif
    buf_idx=(int*)memalloc(sizeof(int)*nMaxSeqLen);
    memset(buf_idx, 0, sizeof(int)*nMaxSeqLen);
#ifndef DISK_BASED
    bufseq=(int*)memalloc(sizeof(int)*nMaxSeqLen);
    memset(bufseq, 0, sizeof(int)*nMaxSeqLen);
#endif



////////////////////////////
////////////////////////////
// Added by Ramin.
#if defined( _PERFORM_COMMON_PREFIX_CHECKING )


    //if( gnArrLargeCount[1]>0 )
	{
		//static int num = 0;
		struct PROJ_DB * ProjDB = NULL;
		Prefix * aPrefix = NULL;

		// For original database intra is the same as inter.
		memcpy( intra, inter, sizeof(struct COUNTER)*gN_ITEMS );

		aPrefix = new Prefix( pDatasetMemMap );

		if( !(*aPrefix).IsEmpty() )
		{
			//num++;
			//printf( "=== Prefix %d ===> ", num );
			//(*aPrefix).Print();

			ProjDB = MakeProjDBFromOrg( pDatasetMemMap, aPrefix, nCount );
			if( ProjDB )
				*pnFreqCount = 1;
			else
				*pnFreqCount = 0;


			proj_db = ProjDB;
			delete aPrefix;
			goto DONE;
		}

		delete aPrefix;
	}

#endif // defined( _PERFORM_COMMON_PREFIX_CHECKING )
////////////////////////////
////////////////////////////
////////////////////////////



// Added by Ramin.
#if defined( _USE_OUTPUT_BUFFER )
    Sequence ** tmpBuf;
	tmpBuf = (Sequence **) memalloc( sizeof(Sequence *) * gN_ITEMS );
#endif

///////////////////////////////
// Changed by Ramin
    for (i=0; i<gN_ITEMS; i++)
    {
        pCnter=inter + i;
        if (pCnter->count>=gSUP)
        {

#if defined( _USE_OUTPUT_BUFFER )
            tmpBuf[i] = OutputPattern( &i, 1, pCnter->count );
#else
            OutputPattern( &i, 1, pCnter->count );
#endif
            //fprintf(gpFreqFile, "(%d) : %f\n", i, double(pCnter->count)/double(gnCustCount));
            //gnArrLargeCount[1]++;
        }
    }
    //fprintf(gpStatusFile, " (%d items) (%.3f sec)\n", gnArrLargeCount[1], GetTimeDiff(1));

///////////////////////////////


    // If there are 1 item frequent seqs.
    if( gnArrLargeCount[1]>0 )
    {
        ResetTimer(1);
        fprintf(gpStatusFile, "\tCreating projection DBs ... "); fflush(gpStatusFile);
        *pnFreqCount = gnArrLargeCount[1];
        n_total_mem+=(nSize=(gnArrLargeCount[1]*sizeof(struct PROJ_DB)));
        proj_db=(struct PROJ_DB*)memalloc(nSize);

        for (nCount=i=0; i<gN_ITEMS; i++)
        {
            if (inter[i].count>=gSUP)
            {
#if defined( _USE_OUTPUT_BUFFER )
                proj_db[nCount].OutputBuf = tmpBuf[i];
#endif
                proj_db[nCount].m_nPatLen=1;
                proj_db[nCount].m_pnPat = (int*)i;

#if defined( _FIND_MAX_SEQS ) && !defined( _DO_NAIVE_APPROACH )
                proj_db[nCount].m_nMaxSup = (*(inter+i)).count;
#else
#if defined (_ANOTHER_CLOSED_APPROACH)
                proj_db[nCount].m_nMaxSup = (*(inter+i)).count;
#else
                proj_db[nCount].m_nMaxSup = inter_freq_idx[i];
#endif
#endif

                n_total_mem+=(nSize=(inter_freq_idx[i]*sizeof(struct PROJ_SEQ)));
                proj_db[nCount].m_pProjSeq = (struct PROJ_SEQ*)memalloc(nSize);
                memset (proj_db[nCount].m_pProjSeq, 0, nSize);

                proj_db[nCount].m_nVer = -1;
                proj_db[nCount].m_nSup = 0;
#if defined( _CALC_I_NUM_OF_ITEMS )
                proj_db[nCount].NumOfItems = 0;
                proj_db[nCount].ItemIsIntra=false;
                proj_db[nCount].Item=i;
#endif

                inter_freq_idx[i] = nCount;
                nCount++;
            } else inter_freq_idx[i]=-1;
        }

        // scan database again, do projection
        dataset = (int*) GetStartOfMap (pDatasetMemMap);

        for (nCount=0; dataset<lastAddr; dataset++)
        {
            nCount++;
            for (; *dataset!=-2; dataset++)
            {
                for (; *dataset!=-1; dataset++)
                {
                    // eat up consecutive identical numbers.
                    while (dataset[0]==dataset[1]) dataset++;
                    i=inter_freq_idx[*dataset];
                    // If this is the last item in Seq, or is not frequent, or an instance of this item has been seen in this seq before.
                    if (dataset[2]==-2 || i<0 || proj_db[i].m_nVer>=nCount) continue;

                    // Pointer to Proj_DB for this item.
                    tempDB = proj_db + i;
                    // Pointer to the next available Seq in this DB.
                    tempSeq = tempDB->m_pProjSeq + tempDB->m_nSup;
                    // Last sequence contributed to this DB.
                    tempDB->m_nVer = nCount;
#if defined( _CALC_I_NUM_OF_ITEMS )
                    (*tempDB).NumOfItems += (lastAddr - dataset);
#endif

#ifdef DISK_BASED
                    tempDB->m_nSup++;

          for (d=dataset+1, buf_idx[0]=(*d), j=1; d[2]!=-2; d++)
					{
            if( *d==*dataset && d[1]!=-1 )
							buf_idx[j++] = *(d+1);
          }
          tempSeq->m_nProjCount = j;
					if (j==1)
					{
						tempSeq->m_ppSeq = (int**) buf_idx[0];
					} else {
						n_total_mem+=(nSize=sizeof(int*)*j);
						tempSeq->m_ppSeq = (int**)memalloc(nSize);
						memcpy (tempSeq->m_ppSeq, buf_idx, nSize);
					}
#else
                    buf_idx[0]=nSeqLen=0; d=dataset+1;
                    if (dataset[1]==-1) { bufseq[nSeqLen++]=*d; d++; }
                    for (j=1; *d!=-2; d++)
                    {
                        for (; *d!=-1; d++)
                        {
                            if (inter_freq_idx[*d]<0) continue;
                            bufseq[nSeqLen++]=*d;
                            if (*d==*dataset && d[1]!=-1) buf_idx[j++] = nSeqLen;
                        }
                        if (nSeqLen==0 || bufseq[nSeqLen-1]!=-1) bufseq[nSeqLen++]=-1;
                    }

                    if (nSeqLen<2) continue;
                    tempDB->m_nSup++;
                    bufseq[nSeqLen++]=-2;
                    tempSeq->m_nProjCount = j;
                    n_total_mem+=(nSize=sizeof(int*)*j);
                    tempSeq->m_ppSeq=(int**)memalloc(nSize);

                    n_total_mem+=(nSize=sizeof(int)*nSeqLen);
                    tempDB->m_nSeqSize=nSize;
                    tempSeq->m_ppSeq[0]=(int*)memalloc(nSize);
                    memcpy(tempSeq->m_ppSeq[0], bufseq, nSize);
                    for (i=1; i<j; i++) tempSeq->m_ppSeq[i]=tempSeq->m_ppSeq[0]+buf_idx[i];
#endif
                }
            }
        }
        fprintf(gpStatusFile, " (%.3f sec)\n", GetTimeDiff(1)); fflush(gpStatusFile);
    }
#if defined( _USE_OUTPUT_BUFFER )
    free( tmpBuf );
#endif
    n_max_mem = n_total_mem;
    //PrintProjDBs(proj_db, *pnFreqCount);

#if defined( _PERFORM_COMMON_PREFIX_CHECKING )
    DONE:
#endif // defined( _PERFORM_COMMON_PREFIX_CHECKING )

    return proj_db;
}

struct PROJ_DB* make_projdb_from_projected_db(const struct PROJ_DB* pDB,
                                              int* pnFreqCount)
{

    struct PROJ_DB* proj_db=NULL;
    int i=0, j=0, k=0, l=0, m=0, nSize=0, nCount=0, nSeqLen=0, nProjCnt=0;
    int *b=0, *d=0, *dataset=0;
    bool bIntra=false, bIndx=false;
    struct PROJ_DB* tempDB=NULL;
    struct PROJ_SEQ* tempSeq=NULL;
    struct COUNTER* pCnter=0;

#if defined( _CALC_I_NUM_OF_ITEMS )
    int *lastAddr = (int*) GetLastAddrOfMap (pDatasetMemMap);
#endif

    *pnFreqCount=0;	// number of frequent items
    //PrintProjDBs(pDB, 1);

    // scan database once, find inter- and intra- element frequent items
    memset(intra, 0, sizeof(struct COUNTER)*gN_ITEMS);
    memset(inter, 0, sizeof(struct COUNTER)*gN_ITEMS);

    for (nCount=0; nCount<pDB->m_nSup;)
    {
        nCount++; nProjCnt=pDB->m_pProjSeq[nCount-1].m_nProjCount;
        for (i=0; i<nProjCnt; i++)
        {
#ifdef DISK_BASED
            if (nProjCnt==1) dataset = (int*) pDB->m_pProjSeq[nCount-1].m_ppSeq;
      else dataset = pDB->m_pProjSeq[nCount-1].m_ppSeq[i];
#else
            dataset = pDB->m_pProjSeq[nCount-1].m_ppSeq[i];
#endif
            // counting intra-element items.
            for (; *dataset!=-1; dataset++)
            {
                // eat up consecutive identical numbers.
                while (dataset[0]==dataset[1]) dataset++;
                pCnter = intra + (*dataset);
                if (pCnter->s_id<nCount)
                {
                    pCnter->count++;
                    pCnter->s_id = nCount;
                }
            }
            // for inter we only need to count the longest instance
            // of a projected sequence. ie. the first one (i==0).
            if (i!=0)
                continue;
            for (dataset++; *dataset!=-2; dataset++)
            {
                // counting inter-element items.
                for (; *dataset!=-1; dataset++)
                {
                    // eat up consecutive identical numbers.
                    while (dataset[0]==dataset[1]) dataset++;
                    pCnter = inter + (*dataset);
                    if (pCnter->s_id<nCount)
                    {
                        pCnter->count++;
                        pCnter->s_id = nCount;
                    }
                }
            }
        }
    }



    for (j=k=i=0; i<gN_ITEMS; i++)
    {
        if (intra[i].count>=gSUP)	j++;
        if (inter[i].count>=gSUP) k++;
    }


////////////////////////////
////////////////////////////
// Added by Ramin.
#if defined( _PERFORM_COMMON_PREFIX_CHECKING )


    //if( (*pDB).m_nSup < (gSUP*4)  ) // To turn common Prefix detection on for small DBs.
	if( (j+k) > 0 )
	{

		//static int num = 0;
		struct PROJ_DB * ProjDB = NULL;
		Prefix * aPrefix = NULL;

		aPrefix = new Prefix( pDB );

		//if( (*aPrefix).NumOfItemSets > 1 )
		if( !(*aPrefix).IsEmpty() )
		{
			//num++;
			//printf( "=== Prefix %d ===> ", num );
			//(*aPrefix).Print();

			ProjDB = MakeProjDB( pDB, aPrefix );
			if( ProjDB )
				*pnFreqCount = 1;
			else
				*pnFreqCount = 0;


			proj_db = ProjDB;
			delete aPrefix;
			goto DONE;
		}

		delete aPrefix;
	}

#endif // defined( _PERFORM_COMMON_PREFIX_CHECKING )
////////////////////////////
////////////////////////////
////////////////////////////

////////////////////////////
////////////////////////////


    //Added by Ramin
#if defined( _USE_OUTPUT_BUFFER )
    if( (j+k) == 0 )
	{
		//fprintf( gpFreqFile, "+===>>>>>>  Right Pattern  " );
		//(*(*pDB).OutputBuf).Print( gpFreqFile );
		EmptyBuffer( aSeqList, (*pDB).OutputBuf );
	}
#endif
#if defined( _FIND_MAX_SEQS ) && !defined( _DO_NAIVE_APPROACH )
    if( (j+k) == 0 )
	{
		Sequence * aSeq = new Sequence( pDB, (*pDB).m_nMaxSup );
		(*MainSeqTree).AddSeq( aSeq );
	}
#endif



    if ((j+k) > 0)
    {
        *pnFreqCount = (j+k);
        n_total_mem+=(nSize=(*pnFreqCount*sizeof(struct PROJ_DB)));
        proj_db=(struct PROJ_DB*)memalloc(nSize);
        memset (inter_freq_idx, -1, sizeof(int)*gN_ITEMS);
        memset (intra_freq_idx, -1, sizeof(int)*gN_ITEMS);
        n_total_mem+=sizeof(int)*((*pnFreqCount*(pDB->m_nPatLen+1))+k);
        for (j=sizeof(int)*(pDB->m_nPatLen+1), nCount=i=0; i<gN_ITEMS; i++)
        {
            if (intra[i].count>=gSUP)
            {
                proj_db[nCount].m_nPatLen = (pDB->m_nPatLen+1);
                proj_db[nCount].m_pnPat = (int*)memalloc(j);
                if (pDB->m_nPatLen == 1)
                {
                    proj_db[nCount].m_pnPat[0] = * pDB->m_pnPat;
                } else {
                    memcpy (proj_db[nCount].m_pnPat, pDB->m_pnPat, j-sizeof(int));
                }
                proj_db[nCount].m_pnPat[pDB->m_nPatLen] = i;

#if defined( _USE_OUTPUT_BUFFER )
                proj_db[nCount].OutputBuf = OutputPattern(proj_db[nCount].m_pnPat, pDB->m_nPatLen+1, intra[i].count );
				(*proj_db[nCount].OutputBuf).Parent = (*pDB).OutputBuf;
				(*(*pDB).OutputBuf).ReferenceCount++;
#else
                OutputPattern(proj_db[nCount].m_pnPat, pDB->m_nPatLen+1, intra[i].count );
#endif
                proj_db[nCount].m_nMaxSup = intra[i].count;
                n_total_mem+=(nSize=intra[i].count*sizeof(struct PROJ_SEQ));
                proj_db[nCount].m_pProjSeq = (struct PROJ_SEQ*)memalloc(nSize);
                memset (proj_db[nCount].m_pProjSeq, 0, nSize);

                proj_db[nCount].m_nVer = -1;
                proj_db[nCount].m_nSup = 0;
#if defined( _CALC_I_NUM_OF_ITEMS )
                proj_db[nCount].NumOfItems = 0;
                proj_db[nCount].ItemIsIntra=true;
                proj_db[nCount].Item=i;
#endif

                intra_freq_idx[i] = nCount;
                nCount++;

            }

            if (inter[i].count>=gSUP)
            {
                proj_db[nCount].m_nPatLen = (pDB->m_nPatLen+2);
                proj_db[nCount].m_pnPat = (int*)memalloc(sizeof(int)+j);
                if (pDB->m_nPatLen == 1)
                {
                    proj_db[nCount].m_pnPat[0] = * pDB->m_pnPat;
                } else {
                    memcpy (proj_db[nCount].m_pnPat, pDB->m_pnPat, j-sizeof(int));
                }
                proj_db[nCount].m_pnPat[pDB->m_nPatLen] = -1;
                proj_db[nCount].m_pnPat[pDB->m_nPatLen+1] = i;

#if defined( _USE_OUTPUT_BUFFER )
                proj_db[nCount].OutputBuf = OutputPattern(proj_db[nCount].m_pnPat, pDB->m_nPatLen+2, inter[i].count );
				(*proj_db[nCount].OutputBuf).Parent = (*pDB).OutputBuf;
				(*(*pDB).OutputBuf).ReferenceCount++;
#else
                OutputPattern(proj_db[nCount].m_pnPat, pDB->m_nPatLen+2, inter[i].count );
#endif
                proj_db[nCount].m_nMaxSup = inter[i].count;
                n_total_mem+=(nSize=inter[i].count*sizeof(struct PROJ_SEQ));
                proj_db[nCount].m_pProjSeq = (struct PROJ_SEQ*)memalloc(nSize);
                memset (proj_db[nCount].m_pProjSeq, 0, nSize);

                proj_db[nCount].m_nVer = -1;
                proj_db[nCount].m_nSup = 0;
#if defined( _CALC_I_NUM_OF_ITEMS )
                proj_db[nCount].NumOfItems = 0;
                proj_db[nCount].ItemIsIntra=false;
                proj_db[nCount].Item=i;
#endif

                inter_freq_idx[i] = nCount;
                nCount++;
            }
        }

#if defined( _USE_STRING_ELEMINATION )
        bool CheckStrings = false;
		if( (*MainSeqTree).IsContained( new Sequence(pDB) ) )
		{
			CheckStrings = true;
		}

#endif // defined( _USE_STRING_ELEMINATION )

        // scan database again, do projection
        for (nCount=0; nCount<pDB->m_nSup;)
        {
            nCount++; nProjCnt=pDB->m_pProjSeq[nCount-1].m_nProjCount;
            for (i=0; i<nProjCnt; i++)
            {
#ifdef DISK_BASED
                if (nProjCnt==1) dataset = (int*) pDB->m_pProjSeq[nCount-1].m_ppSeq;
				else dataset = pDB->m_pProjSeq[nCount-1].m_ppSeq[i];
#else
                dataset = pDB->m_pProjSeq[nCount-1].m_ppSeq[i];
#endif


#if defined( _USE_STRING_ELEMINATION )
                if( CheckStrings )
				{
					if( (*MainSeqTree).IsContained( pDB, dataset ) )
					{
						//Sequence * aS = new Sequence( pDB, dataset );
						//(*aS).Print();
						//printf( " Sequence Eleminated.\n" );
						continue;
					}
				}
#endif // defined( _USE_STRING_ELEMINATION )

                // counting intra-element items.
                for (; *dataset>=0; dataset++)
                {
                    // eat up consecutive identical numbers.
                    while (dataset[0]==dataset[1]) dataset++;

                    j=intra_freq_idx[*dataset];
                    if (dataset[2]==-2 || j<0 || proj_db[j].m_nVer>=nCount) continue;

                    tempDB = proj_db + j;
                    tempSeq = tempDB->m_pProjSeq + tempDB->m_nSup;
                    tempDB->m_nVer = nCount;

#if defined( _CALC_I_NUM_OF_ITEMS )
                    (*tempDB).NumOfItems += (lastAddr - dataset );
                    /*if ((*tempDB).NumOfItems == 1) {
                        printf("INTRAERROR\n");
                        printf("%d\n", lastAddr-dataset);
                        exit(1);
                    }*/
#endif

                    for (buf_idx[0]= * (dataset+1), l=1, j=i; j<nProjCnt; j++)
                    {
                        if (j==i) d=dataset+1; else d=pDB->m_pProjSeq[nCount-1].m_ppSeq[j];
                        while (*d!=-1 && d[2]!=-2 && *d!=*dataset) d++;
                        if (d[2]!=-2 && *d==*dataset && d[1]!=-1) buf_idx[l++]= *(d+1);
                    }
#ifdef DISK_BASED
                    tempDB->m_nSup++;
					tempSeq->m_nProjCount = l;
					if (l==1)
					{
						tempSeq->m_ppSeq = (int**) buf_idx[0];
					} else {
						n_total_mem+=(nSize=sizeof(int*)*l);
						tempSeq->m_ppSeq = (int**)memalloc(nSize);
						memcpy (tempSeq->m_ppSeq, buf_idx, nSize);
					}
#else
                    buf_idx[0]=nSeqLen=0; bIntra=true; d=dataset+1;
                    for (k=l, j=l=1; *d!=-2; d++)
                    {
                        for (bIndx=false; *d!=-1; d++)
                        {
                            if (j<k && (int*)(buf_idx[j])==d)
                            {
                                bIntra=bIndx=true; j++; buf_idx[l++]=nSeqLen;
                            }
                            if (bIntra)
                            {
                                if (intra_freq_idx[*d]<0 && inter_freq_idx[*d]<0) continue;
                            } else if (inter_freq_idx[*d]<0) continue;
                            bufseq[nSeqLen++]=*d; bIndx=false;
                        }
                        bIntra=false; if (bIndx) l--;
                        if (nSeqLen==0 || bufseq[nSeqLen-1]!=-1) bufseq[nSeqLen++]=-1;
                    }
                    if (nSeqLen<2) continue;
                    tempDB->m_nSup++;
                    bufseq[nSeqLen++]=-2;
                    tempSeq->m_nProjCount = l;
                    n_total_mem+=(nSize=sizeof(int*)*l);
                    tempSeq->m_ppSeq=(int**)memalloc(nSize);

                    n_total_mem+=(nSize=sizeof(int*)*nSeqLen);
                    tempDB->m_nSeqSize=nSize;
                    tempSeq->m_ppSeq[0]=(int*)memalloc(nSize);
                    memcpy(tempSeq->m_ppSeq[0], bufseq, nSize);
                    for (j=1; j<l; j++) tempSeq->m_ppSeq[j]=tempSeq->m_ppSeq[0]+buf_idx[j];
#endif
                } // end for counting intra-element items.


                // for inter we only need to work with the longest instance
                // of a projected sequence. ie. the first one (i==0).
                if (i!=0) continue;
                for (dataset++; *dataset!=-2; dataset++)
                {
                    // counting inter-element items.
                    for (; *dataset!=-1; dataset++)
                    {
                        // eat up consecutive identical numbers.
                        while (dataset[0]==dataset[1]) dataset++;
                        j=inter_freq_idx[*dataset];
                        if (dataset[2]==-2 || j<0 || proj_db[j].m_nVer>=nCount) continue;

                        tempDB = proj_db + j;
                        tempSeq = tempDB->m_pProjSeq + tempDB->m_nSup;
                        tempDB->m_nVer = nCount;

#if defined( _CALC_I_NUM_OF_ITEMS )
                        (*tempDB).NumOfItems += (lastAddr - dataset);
                        /*if ((*tempDB).NumOfItems == 1) {
                            printf("INTERERROR\n");
                            printf("%d\n", lastAddr-dataset);
                            exit(1);
                        }*/
#endif

                        for (d=dataset+1, buf_idx[0]=*(d), l=1; d[2]!=-2; d++)
                        {
                            if (*d==*dataset && d[1]!=-1) buf_idx[l++] = *(d+1);
                        }
#ifdef DISK_BASED
                        tempDB->m_nSup++;
						tempSeq->m_nProjCount = l;
						if (l==1)
						{
							tempSeq->m_ppSeq = (int**) buf_idx[0];
						} else {
							n_total_mem+=(nSize=sizeof(int*)*l);
							tempSeq->m_ppSeq = (int**)memalloc(nSize);
							memcpy (tempSeq->m_ppSeq, buf_idx, nSize);
						}
#else
                        buf_idx[0]=nSeqLen=0; d=dataset+1;
                        for (k=l, j=l=1; *d!=-2; d++)
                        {
                            for (bIndx=false; *d!=-1; d++)
                            {
                                if (j<k && (int*)(buf_idx[j])==d) { bIndx=true; j++; buf_idx[l++]=nSeqLen; }
                                if (inter_freq_idx[*d]<0) continue;
                                bufseq[nSeqLen++]=*d; bIndx=false;
                            }
                            if (bIndx) l--;
                            if (nSeqLen==0 || bufseq[nSeqLen-1]!=-1) bufseq[nSeqLen++]=-1;
                        }
                        if (nSeqLen<2) continue;
                        tempDB->m_nSup++;
                        bufseq[nSeqLen++]=-2;
                        tempSeq->m_nProjCount = l;
                        n_total_mem+=(nSize=sizeof(int*)*l);
                        tempSeq->m_ppSeq=(int**)memalloc(nSize);

                        n_total_mem+=(nSize=sizeof(int*)*nSeqLen);
                        tempDB->m_nSeqSize=nSize;
                        tempSeq->m_ppSeq[0]=(int*)memalloc(nSize);
                        memcpy(tempSeq->m_ppSeq[0], bufseq, nSize);
                        for (j=1; j<l; j++) tempSeq->m_ppSeq[j]=tempSeq->m_ppSeq[0]+buf_idx[j];
#endif
                    }
                } // end for counting inter-element items.
            } // end of all projection instances of a sequence.
        } // end of all sequences.
    } // enf if number of projections is greater than 0.

#if defined( _PERFORM_COMMON_PREFIX_CHECKING )
    DONE:
#endif // defined( _PERFORM_COMMON_PREFIX_CHECKING )

    if (n_max_mem<n_total_mem) n_max_mem = n_total_mem;
    if (pDB->m_nPatLen > 1)
    {
        n_total_mem-=(pDB->m_nPatLen*sizeof(int));
        freemem ((void**) &(pDB->m_pnPat));
    }
    for (i=0; i<pDB->m_nSup; i++)
    {
#ifndef DISK_BASED
        n_total_mem-=pDB->m_nSeqSize;
        freemem ((void**) &(pDB->m_pProjSeq[i].m_ppSeq[0]));
#endif

        if (pDB->m_pProjSeq[i].m_nProjCount > 1)
        {
            n_total_mem-=(pDB->m_pProjSeq[i].m_nProjCount*sizeof(int*));
            freemem ((void**) &(pDB->m_pProjSeq[i].m_ppSeq));
        }
    }
    n_total_mem-=(pDB->m_nMaxSup*sizeof(struct PROJ_SEQ));
    freemem ((void**) &(pDB->m_pProjSeq));

    // PrintProjDBs(proj_db, *pnFreqCount);
    return proj_db;
}

//this part is copied from make_projdb_from_projected_db's tail part.
#if defined (_ANOTHER_CLOSED_APPROACH)
void clean_projcted_db(const struct PROJ_DB* pDB, int* pnFreqCount)
{
    int i;

    if (n_max_mem<n_total_mem) n_max_mem = n_total_mem;
    if (pDB->m_nPatLen > 1)
    {
        n_total_mem-=(pDB->m_nPatLen*sizeof(int));
        freemem ((void**) &(pDB->m_pnPat));
    }
    for (i=0; i<pDB->m_nSup; i++)
    {
#ifndef DISK_BASED
        n_total_mem-=pDB->m_nSeqSize;
        freemem ((void**) &(pDB->m_pProjSeq[i].m_ppSeq[0]));
#endif

        if (pDB->m_pProjSeq[i].m_nProjCount > 1)
        {
            n_total_mem-=(pDB->m_pProjSeq[i].m_nProjCount*sizeof(int*));
            freemem ((void**) &(pDB->m_pProjSeq[i].m_ppSeq));
        }
    }
    n_total_mem-=(pDB->m_nMaxSup*sizeof(struct PROJ_SEQ));
    freemem ((void**) &(pDB->m_pProjSeq));
}
#endif

//////////////////////////////////////////////////////////////////////
// Local functions.
//////////////////////////////////////////////////////////////////////
void PrintProjDBs(const struct PROJ_DB *proj_db, const int nCount){

    int i=0, j=0, k=0, l=0, nProjCnt=0, *dataset=0;

    printf("\nProjected databases:\n");
    for (i=0; i<nCount; i++){
        printf("Proj. DB. for (");
        if (proj_db[i].m_nPatLen == 1) printf("%d", *(proj_db[i].m_pnPat));
        else for (j=0; j<proj_db[i].m_nPatLen; j++) {
                if (proj_db[i].m_pnPat[j]==-1) printf(")");
                else if (j>0 && proj_db[i].m_pnPat[j-1]==-1) printf(" (%d", proj_db[i].m_pnPat[j]);
                else if (j>0) printf(" %d", proj_db[i].m_pnPat[j]);
                else printf("%d", proj_db[i].m_pnPat[j]);
            }
        printf("\n ");

#if defined( _CALC_I_NUM_OF_ITEMS )
        printf( "NumOfItems = %d m_nSupport= %d  maxSupport= %d totalmem= %d maxmem= %d\n",
                proj_db[i].NumOfItems, proj_db[i].m_nSup, proj_db[i].m_nMaxSup, n_total_mem, n_max_mem );
        /*if (proj_db[i].NumOfItems == 1) {
            printf("WRONGPRINT\n");
            exit(1);
        }*/
#endif

        return;

        for (j=0; j<proj_db[i].m_nSup; j++){
            nProjCnt = proj_db[i].m_pProjSeq[j].m_nProjCount;
            for (k=0; k<nProjCnt; k++){
#ifdef DISK_BASED
                if (nProjCnt==1) dataset = (int*) proj_db[i].m_pProjSeq[j].m_ppSeq;
				else dataset = proj_db[i].m_pProjSeq[j].m_ppSeq[k];
#else
                dataset = proj_db[i].m_pProjSeq[j].m_ppSeq[k];
#endif
                for(l=0; dataset[l]!=-2; l++){
                    if (dataset[l]==-1) printf(")");
                    else if (l>0 && dataset[l-1] == -1) printf(" (%d", dataset[l]);
                    else if (l>0) printf(" %d", dataset[l]);
                    else printf("%d", dataset[l]);
                } // end of an instance for a projected sequence.
                printf("\n ");
            } // end of all instances for a projected sequence.
            printf("\n ");
        } // end of all projected sequences.
    }
}

#if defined( _USE_OUTPUT_BUFFER )

//inline Sequence * OutputPattern( const int *const pat, const int nPatLen, const int nSup )
Sequence * OutputPattern( const int *const pat, const int nPatLen, const int nSup )
{
	int i=0, l=0;

	Sequence * aSeq = new Sequence( (int *) pat, (int) nPatLen, (int) nSup );
	/////////////////

#if defined( _WRITE_FREQUENT_FILE )
  for (; i<nPatLen; i++){
	  fprintf(gpFreqFile, "(%d", pat[i]);
    for (i++, l++; (i<nPatLen) && (pat[i]!=-1); i++){
      l++;
      fprintf (gpFreqFile, " %d", pat[i]);
    }
    fprintf(gpFreqFile, ") ");
  }
	fprintf(gpFreqFile, ": %f\n", double(nSup)/double(gnCustCount));
#else
  for( ; i<nPatLen; i++ )
		if( pat[i] != -1 )
			l++;

#endif
	gnArrLargeCount[l]++;

	return aSeq;
}

#else // defined( _USE_OUTPUT_BUFFER )

//inline bool OutputPattern( const int *const pat, const int nPatLen, const int nSup )
bool OutputPattern( const int *const pat, const int nPatLen, const int nSup )
{
    bool Result = true;
    int i=0, l=0;

    // Added by Ramin
#if defined( _FIND_CLOSED_SEQS )
    Sequence * aSeq = new Sequence( (int *) pat, (int) nPatLen, (int) nSup );

	Result = (*aSeqList).AddIfClosedSeq( aSeq );

	if( !Result )
		delete aSeq;

#elif defined( _FIND_MAX_SEQS ) && defined( _DO_NAIVE_APPROACH )
    //Sequence * aSeq = new Sequence( proj_db );
	Sequence * aSeq = new Sequence( (int *)pat, (int) nPatLen, (int) nSup );
	(*MainSeqTree).AddSeq( aSeq );

#endif
    /////////////////

#if defined( _WRITE_FREQUENT_FILE )
    for (; i<nPatLen; i++){
	  fprintf(gpFreqFile, "(%d", pat[i]);
    for (i++, l++; (i<nPatLen) && (pat[i]!=-1); i++){
      l++;
      fprintf (gpFreqFile, " %d", pat[i]);
    }
    fprintf(gpFreqFile, ") ");
  }
	fprintf(gpFreqFile, ": %f  %d\n", double(nSup)/double(gnCustCount), nSup);
#else
    for( ; i<nPatLen; i++ )
        if( pat[i] != -1 )
            l++;

#endif
    gnArrLargeCount[l]++;

    return Result;
}

#endif // defined( _USE_OUTPUT_BUFFER )

//////////////////////////////////////////////////////////////////////
// END

// Added by Ramin
void Test()
{
    int * dataset = (int*) GetStartOfMap (pDatasetMemMap);
    int * lastAddr = (int*) GetLastAddrOfMap (pDatasetMemMap);
    int * RecStart;
    FILE *testFile = NULL;
    testFile = file_open( "Test.txt", "w" );
    long NumOfRecs = 0;
    long NumOfItems = 0;
    long NumOfItemSets = 0;

    for( int nCount=0; dataset<lastAddr; dataset++, nCount++ )
    {
        RecStart = dataset;
        NumOfRecs++;
        // For each sequence
        for (; *dataset!=-2; dataset++)
        {
            NumOfItemSets++;
            // For each Itemset
            for (; *dataset!=-1; dataset++)
            {
                NumOfItems++;
                fprintf( testFile, " %d ", *dataset );
            }
            fprintf( testFile, " | " );
        }
        fprintf( testFile, "\n" );
    }

    fprintf( testFile, "Number of records = %ld\n", NumOfRecs );
    fprintf( testFile, "Number of ItemSets = %ld\n", NumOfItemSets );
    fprintf( testFile, "Number of Items = %ld\n", NumOfItems );
    fprintf( testFile, "Average number of ItemSets in a recored = %ld\n", NumOfItemSets / NumOfRecs );
    fprintf( testFile, "Average number of Items in an itemset = %ld\n", NumOfItems / NumOfItemSets );

    fclose( testFile );
    exit( 0 );
}
