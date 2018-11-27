#include <assert.h>
#include "Global.h"
#include "ProjDB.h"

#if !defined( _FIND_MAX_SEQS ) && !defined( _FIND_FREQUENT_SEQS )

//#include "SequenceList.h"

void RTATest( const struct PROJ_DB * pDB );

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////

#if defined( _USE_OUTPUT_BUFFER )

void EmptyBuffer( SequenceList * aList, Sequence * aSeq )
{
	Sequence * aBuf;
	Sequence * aParent;
	int tmpSup = 0;

	if( n_max_mem < n_total_mem )
		n_max_mem = n_total_mem;

	aBuf = aSeq;
	while( aBuf != NULL )
	{
		aParent = (*aBuf).Parent;
		(*aBuf).ReferenceCount--;
		if( (*aBuf).ReferenceCount <= 0 )
		{
			if( (*aBuf).Support > tmpSup )
			{
				tmpSup = (*aBuf).Support;
				(*aList).AddIfClosedSeq( aBuf );
			} else {
				delete aBuf;
			}

		} else
			break;
		aBuf = aParent;
	}
	
	// Ignor parent sequences that have the same support, when traversing through their other children.
	while( aBuf != NULL && (*aBuf).Support <= tmpSup )
	{
		(*aBuf).Support = -1;
		aBuf = (*aBuf).Parent;
	}

}

#endif


/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////


void main1( int argc, char** argv )
{
	printf( "This is SequenceList running!!!\n" );


}



//************************
//************************


Sequence::Sequence()
{
	StartPtr = NULL;
	Support = 0;
	Len = 0;
#if defined( _USE_OUTPUT_BUFFER )
	Parent = NULL;
	ReferenceCount = 0;
#endif
}

// No last -1, and -2 are presented.
Sequence::Sequence( int * Ptr, int Len, int Sup )
{
	int tmp;
	Support = Sup;
	(*this).Len = Len;
#if defined( _USE_OUTPUT_BUFFER )
	Parent = NULL;
	ReferenceCount = 0;
#endif

	tmp = (*this).Len * sizeof( int );
	StartPtr = (int*) memalloc( tmp );
	n_total_mem += ( tmp  + sizeof( *this ) );
	memcpy( StartPtr, Ptr, tmp );
}

inline bool Sequence::EndOfSequence( int * aPtr )
{
	return (aPtr-StartPtr >= Len);
}

inline bool Sequence::EndOfItemSet( int * aPtr )
{
	return (*aPtr==-1) || (aPtr-StartPtr >= Len);
}

inline int Sequence::NumOfItemSets()
{
	int * ItemPtr;
	int Result = 0;

	if( Len>0 )
		Result = 1;

	for( ItemPtr = StartPtr; !EndOfSequence( ItemPtr ); ItemPtr++ )
	{
		if( *ItemPtr == -1 )
			Result++;
	}
	return Result;
}

// Returns the address of the next itemset in aSeq, if itemset pointed to by ThisStart
// is the subset of the itemset pointed to by SeqStart.
inline int * Sequence::IsItemSetSubsetOf( int * ThisStart, int * SeqStart, Sequence * aSeq )
{
	int * aSeqPtr = SeqStart;
	int * thisPtr = ThisStart;

	while( !(*aSeq).EndOfSequence(aSeqPtr) )
	{
		if( *aSeqPtr == *thisPtr )
		{
			thisPtr++;
			if( EndOfItemSet(thisPtr) )
			{
				while( !(*aSeq).EndOfItemSet(aSeqPtr) )
					aSeqPtr++;
				return ++aSeqPtr;
			}
		}

		aSeqPtr++;
		if( (*aSeq).EndOfItemSet(aSeqPtr) )
			thisPtr = ThisStart;
	}
	return NULL;
}

// Returns true if this sequence is the closed subset of aSeq.
bool Sequence::IsClosedSubsetOf( Sequence * aSeq )
{
	int * aSeqPtr = (*aSeq).StartPtr;
	int * thisPtr = StartPtr;

	if( Len == 0 )
		return true;

	if( (*aSeq).Len >= Len && (*aSeq).Len > 0 && (*aSeq).Support >= Support )
	{
		while( !(*aSeq).EndOfSequence(aSeqPtr) )
		{
			aSeqPtr = IsItemSetSubsetOf( thisPtr, aSeqPtr, aSeq );

			if( aSeqPtr == NULL )
				return false;
			else {
				while( !EndOfItemSet(thisPtr) )
					thisPtr++;
				if( EndOfSequence(++thisPtr) )
					return true;
			}
		}

	}

	return false;
}

void Sequence::Print( FILE *aFile )
{
	int * TmpPtr = NULL;

	fprintf( aFile, " <" );


	TmpPtr = StartPtr;

	if( TmpPtr!=NULL )
		fprintf( aFile, " (" );

	while( TmpPtr != NULL && TmpPtr-StartPtr <= Len )
	{
		if( TmpPtr-StartPtr == Len )
			fprintf( aFile, ") " );
		else if( *TmpPtr == -1 )
		{
				fprintf( aFile, ")  (" );
		} else {
			fprintf( aFile, " %d ", *TmpPtr );
		}

		TmpPtr++;

	}

	fprintf( aFile, "> " );

	fprintf( aFile, "  Support = %d\n", Support );
}

Sequence::~Sequence()
{
	int tmp;

	tmp = (*this).Len * sizeof( int );
	n_total_mem -= ( tmp  + sizeof( *this ) );
	freemem( (void**) &StartPtr );
}

//************************
//************************

//************************
//************************
#define _CONSIDER_LEN

inline bool SeqGreater( Sequence * a, Sequence * b )
{
#if defined( _CONSIDER_LEN )
	if( (*a).Support > (*b).Support )
		return true;
	else if( (*a).Support < (*b).Support )
		return false;
	else if( (*a).Len > (*b).Len )
		return true;
	else if( (*a).Len < (*b).Len )
		return false;

	for( int i=0; i<(*a).Len; i++ )
	{
		if( *((*a).StartPtr+i) > *((*b).StartPtr+i) )
			return true;
		if( *((*a).StartPtr+i) < *((*b).StartPtr+i) )
			return false;
	}
	return false;
#else
	return (*a).Support > (*b).Support;
#endif
}

inline bool SeqGreaterEqual( Sequence * a, Sequence * b )
{
#if defined( _CONSIDER_LEN )
	if( (*a).Support > (*b).Support )
		return true;
	else if( (*a).Support < (*b).Support )
		return false;
	else
		return (*a).Len >= (*b).Len;
#else
	return (*a).Support >= (*b).Support;
#endif
}

inline bool SeqEqual( Sequence * a, Sequence * b )
{
#if defined( _CONSIDER_LEN )
	return ( (*a).Support == (*b).Support ) && ( (*a).Len == (*b).Len );
#else
	return (*a).Support == (*b).Support;
#endif
}

#if _NEW_SEQUENCE_LIST > 0 && _NEW_SEQUENCE_LIST < 20
inline bool LenGreater( Sequence * a, Sequence * b )
{
	if( (*a).Len > (*b).Len )
		return true;
	else if( (*a).Len < (*b).Len )
		return false;

	for( int i=0; i<(*a).Len; i++ )
	{
		if( *((*a).StartPtr+i) > *((*b).StartPtr+i) )
			return true;
		if( *((*a).StartPtr+i) < *((*b).StartPtr+i) )
			return false;
	}
	return false;
}
#endif // _NEW_SEQUENCE_LIST > 0 && _NEW_SEQUENCE_LIST < 20


#if defined( _NEW_SEQUENCE_LIST )

#if _NEW_SEQUENCE_LIST < 20

SequenceList::SequenceList()
{
	NumOfAdds = 0;
	NumOfDels = 0;
	NumOfAddIfClosed = 0;
	NumIsClosedSubsetOf = 0;
}


inline void SequenceList::AddSeq( Sequence * aSeq )
{
	NumOfAdds++;

#if _NEW_SEQUENCE_LIST > 0
	SeqMap::iterator Itrtr;
	SeqList * tmpSeqList;

	Itrtr = List.find( (*aSeq).Support );
	if( Itrtr != List.end() )
	{
		tmpSeqList = (*Itrtr).second;
	} else {
		tmpSeqList = new SeqList();
		List.insert( SeqMap::value_type( (*aSeq).Support, tmpSeqList ) );
	}
	(*tmpSeqList).push_back( aSeq );
	// To keep the List vector sorted.
	inplace_merge( (*tmpSeqList).begin(), (*tmpSeqList).end()-1, (*tmpSeqList).end(), LenGreater );
#else //_NEW_SEQUENCE_LIST > 0
	List.push_back( aSeq );
	// To keep the List vector sorted.
	inplace_merge( List.begin(), List.end()-1, List.end(), SeqGreater );
#endif //_NEW_SEQUENCE_LIST > 0

}

inline SeqList::iterator SequenceList::Del( SeqList::iterator anItr )
{
	NumOfDels++;
#if _NEW_SEQUENCE_LIST > 0
#else //_NEW_SEQUENCE_LIST > 0
	return List.erase( anItr );
#endif //_NEW_SEQUENCE_LIST > 0
}

// Tries to add aSeq to the list if non of the list elements
// is a closed superset of aSeq.
// Sequences in the list that are closed subset of aSeq
// will be deleted from the list.
// Returns true if aSeq is added.
bool SequenceList::AddIfClosedSeq( Sequence * aSeq )
{
	NumOfAddIfClosed++;

#if _NEW_SEQUENCE_LIST > 0
	SeqMap::iterator Itrtr;
	SeqList::iterator ListItrtr;
	SeqList * aList;

	for( Itrtr = List.begin(); Itrtr != List.end() && (*Itrtr).first>=(*aSeq).Support; Itrtr++ )
	{
		aList = (*Itrtr).second;
		for( ListItrtr = (*aList).begin(); ListItrtr != (*aList).end() && (*(*ListItrtr)).Len>=(*aSeq).Len; )
		{
			NumIsClosedSubsetOf++;
			if( (*aSeq).IsClosedSubsetOf( (*ListItrtr) ) )
				return false;
			if( (*aSeq).Support == (*Itrtr).first && (*(*ListItrtr)).Len == (*aSeq).Len )
			{
				NumIsClosedSubsetOf++;
				if( (*(*ListItrtr)).IsClosedSubsetOf( aSeq ) )
				{
					delete (*ListItrtr);

					NumOfDels++;
					
					ListItrtr = (*aList).erase( ListItrtr );
				} else
					ListItrtr++;
			} else
				ListItrtr++;
		}
	}

	// Go to the list with equal support with aSeq.
	if( Itrtr != List.begin() ) 
	{
		Itrtr--; 
		if( (*(Itrtr)).first != (*aSeq).Support )
			Itrtr++; 
	}

	while( Itrtr != List.end() ) 
	{
		aList = (*Itrtr).second;
		for( ListItrtr = (*aList).end()-1; ListItrtr >= (*aList).begin() && (*(*ListItrtr)).Len<=(*aSeq).Len; )
		{
			NumIsClosedSubsetOf++;
			if( (*(*ListItrtr)).IsClosedSubsetOf( aSeq ) )
			{
				delete (*ListItrtr);
				NumOfDels++;
					
				ListItrtr = (*aList).erase( ListItrtr ) - 1;
			} else
				ListItrtr--;
		}
		Itrtr++;
	}

	AddSeq( aSeq );
	return true;
#else //_NEW_SEQUENCE_LIST > 0
	SeqList::iterator Itrtr;

	Itrtr = List.begin();

	while( Itrtr != List.end() && SeqGreaterEqual( (*Itrtr), aSeq ) ) 
	{
		NumIsClosedSubsetOf++;
		if( (*aSeq).IsClosedSubsetOf( (*Itrtr) ) )
			return false;

		if( SeqEqual( aSeq, (*Itrtr) ) )
		{
			NumIsClosedSubsetOf++;
			if( (*(*Itrtr)).IsClosedSubsetOf( aSeq ) )
			{
				delete (*Itrtr);

				Itrtr = Del( Itrtr );
			} else
				Itrtr++;
		} else
			Itrtr++;
	}

	while( Itrtr != List.end() ) 
	{
		NumIsClosedSubsetOf++;
		if( (*(*Itrtr)).IsClosedSubsetOf( aSeq ) )
		{
			delete (*Itrtr);

			Itrtr = Del( Itrtr );
		} else
			Itrtr++;
	}

	AddSeq( aSeq );
	return true;
#endif //_NEW_SEQUENCE_LIST > 0
}

void SequenceList::Print( FILE *aFile )
{
	int ListSize = 0;
	fprintf( aFile, "\n" );
#if _NEW_SEQUENCE_LIST > 0
	SeqMap::iterator Itrtr;
	SeqList::iterator ListItrtr;
	SeqList * aList;

	for( Itrtr = List.begin(); Itrtr != List.end(); Itrtr++)
	{
		aList = (*Itrtr).second;
		for( ListItrtr = (*aList).begin(); ListItrtr != (*aList).end(); ListItrtr++)
		{
			(*(*ListItrtr)).Print( aFile );
			ListSize++;
		}
	}
#else //_NEW_SEQUENCE_LIST > 0
	SeqList::iterator Itrtr;

	for( Itrtr = List.begin(); Itrtr != List.end(); Itrtr++)
	{
		(*(*Itrtr)).Print( aFile );
		//fprintf( aFile, "\n" );
	}
	ListSize = List.size();

#endif //_NEW_SEQUENCE_LIST > 0
	fprintf( aFile, "\n" );
	fprintf( aFile, "# of elements added %d\n", NumOfAdds );
	fprintf( aFile, "# of elements deleteded %d\n", NumOfDels );
	fprintf( aFile, "# of times AddIfClosedSeq called %d\n", NumOfAddIfClosed );
	fprintf( aFile, "# of times IsClosedSubsetOf called %d\n", NumIsClosedSubsetOf );
	fprintf( aFile, "# of elements in the list %d\n", ListSize );
}

SequenceList::~SequenceList()
{
#if _NEW_SEQUENCE_LIST > 0
	SeqMap::iterator Itrtr;
	SeqList::iterator ListItrtr;
	SeqList * aList;

	for( Itrtr = List.begin(); Itrtr != List.end(); Itrtr++)
	{
		aList = (*Itrtr).second;
		for( ListItrtr = (*aList).begin(); ListItrtr != (*aList).end(); ListItrtr++)
		{
			delete (*ListItrtr);
		}
		delete aList;
	}
#else //_NEW_SEQUENCE_LIST > 0
	SeqList::iterator Itrtr;

	for( Itrtr = List.begin(); Itrtr != List.end(); Itrtr++)
		delete (*Itrtr);
#endif //_NEW_SEQUENCE_LIST > 0
}

//************************
//************************
#else // _NEW_SEQUENCE_LIST < 20
//************************
//************************

//************************************
//************************************
//************************************

bool TreeNodeLess( TreeNode * a, TreeNode * b )
{
	if( (*a).ItemIsIntra == (*b).ItemIsIntra )
		return (*a).Item < (*b).Item; 
	else if( (*a).ItemIsIntra && !(*b).ItemIsIntra )
		return true;
	else
		return false;
}

int TreeNodeCompare( TreeNode ** a, TreeNode ** b )
{
	if( (*(*a)).ItemIsIntra == (*(*b)).ItemIsIntra )
		if( (*(*a)).Item == (*(*b)).Item )
			return 0;
		else if( (*(*a)).Item < (*(*b)).Item )
			return -1;
		else
			return 1;
	else if( (*(*a)).ItemIsIntra && !(*(*b)).ItemIsIntra )
		return -1;
	else
		return 1;
}

TreeNode::TreeNode( int anItem, bool IsIntra, int Sup, TreeNode * aParent )
{
	Children = NULL;
	Parent = aParent;
	ItemsetNumber = 0;
	Item = anItem;
	ItemIsIntra = IsIntra;
	Support = Sup;
}

inline TreeNode * TreeNode::FindChild( int anItem, bool Intra )
{
	TreeNode ** Res;
	TreeNode * tmp = new TreeNode( anItem, Intra );

	Res = (TreeNode **) bsearch( &tmp, (*Children).begin(), (*Children).size(), sizeof( TreeNode *), (int (*)(const void*, const void*))TreeNodeCompare );

	delete tmp;

	if( Res )
		return (*Res);
	else
		return NULL;
}

inline TreeNode * TreeNode::FindChild( TreeNode * Child )
{
	TreeNode ** Res;

	Res = (TreeNode **) bsearch( &Child, (*Children).begin(), (*Children).size(), sizeof( TreeNode *), (int (*)(const void*, const void*))TreeNodeCompare );

	if( Res )
		return (*Res);
	else
		return NULL;
}

inline void TreeNode::DelChild( TreeNode * Child )
{
	TreeNode ** Res;

	if( Children==NULL )
		return;

	if( (*Children).size() == 1 )
	{
		Res = (*Children).begin();
	} else {
		Res = (TreeNode **) bsearch( &Child, (*Children).begin(), (*Children).size(), sizeof( TreeNode *), (int (*)(const void*, const void*))TreeNodeCompare );
	}

	if( Res )
	{
		(*Children).erase( (Res) );
	}

}

TreeNode * TreeNode::AddChild( TreeNode * Child ) 
{ 
	TreeNode * Result = NULL;

	if( Children == NULL )
	{
		Children = new NodeVector();
		(*Children).push_back( Child );
		Result = Child;
		(*Child).Parent = this;
		if( (*Child).ItemIsIntra )
			(*Child).ItemsetNumber = ItemsetNumber;
		else
			(*Child).ItemsetNumber = ItemsetNumber + 1;
	} else {
		Result = FindChild( Child );
		if( Result == NULL )
		{
			Result = Child;
			(*Children).push_back( Child ); 
			// To keep the children vector sorted.
			inplace_merge( (*Children).begin(), (*Children).end()-1, (*Children).end(), TreeNodeLess );
			(*Child).Parent = this;
			if( (*Child).ItemIsIntra )
				(*Child).ItemsetNumber = ItemsetNumber;
			else
				(*Child).ItemsetNumber = ItemsetNumber + 1;
		} else {
			if( (*Child).Support > (*Result).Support )
				(*Result).Support = (*Child).Support;

			delete Child;

		}
	}

	return Result;
}

inline int TreeNode::NumOfChildren()
{
	if( Children==NULL )
		return 0;
	else
		return (*Children).size();
}

inline int TreeNode::MaxChildSupport()
{
	int MaxChildSup = 0;
  NodeVector::iterator Itrtr;

	if( Children==NULL )
		return 0;

	for( Itrtr = (*Children).begin(); Itrtr != (*Children).end(); Itrtr++ )
		if( (*(*Itrtr)).Support > MaxChildSup )
			MaxChildSup = (*(*Itrtr)).Support;

	return MaxChildSup;
}

bool TreeNode::LastItemOfSequence()
{
	if( Children==NULL )
		return true;

	if( MaxChildSupport() < Support )
		return true;

	return false;
}

void TreeNode::PrintRules( FILE * aFile, int * SeqBuf, int SeqLen, SequenceList * aSeqTree, SeqList * aSeqList, int NumOfItems )
{
	Sequence * aSeq;
	int MaxChildSupport = 0;
  NodeVector::iterator Itrtr;

	SeqBuf[ SeqLen ] = Item;

	if( Children != NULL )
	{
		for( Itrtr = (*Children).begin(); Itrtr != (*Children).end(); Itrtr++ )
		{
			if( !(*(*Itrtr)).ItemIsIntra )
			{
				SeqBuf[ SeqLen + 1 ] = -1;
				(*(*Itrtr)).PrintRules( aFile, SeqBuf, SeqLen + 2, aSeqTree, aSeqList, NumOfItems + 1 );
			} else {
				(*(*Itrtr)).PrintRules( aFile, SeqBuf, SeqLen + 1, aSeqTree, aSeqList, NumOfItems + 1 );
			}
			if( (*(*Itrtr)).Support > MaxChildSupport )
			{
				MaxChildSupport = (*(*Itrtr)).Support;
			} 
		}
		if( MaxChildSupport < Support )
		{
			aSeq = new Sequence( SeqBuf, SeqLen + 1, Support );
			gnResSizeCount[NumOfItems]++;

			if( aSeqList==NULL )
			{
				(*aSeq).Print( aFile );
				delete aSeq;
			} else {
				(*aSeqList).push_back( aSeq );
				// To keep the List vector sorted.
				inplace_merge( (*aSeqList).begin(), (*aSeqList).end()-1, (*aSeqList).end(), SeqGreater );
			}
			(*aSeqTree).Size++;
		}
	} else {
		aSeq = new Sequence( SeqBuf, SeqLen + 1, Support );
		gnResSizeCount[NumOfItems]++;

		if( aSeqList==NULL )
		{
			(*aSeq).Print( aFile );
			delete aSeq;
		} else {
			(*aSeqList).push_back( aSeq );
			// To keep the List vector sorted.
			inplace_merge( (*aSeqList).begin(), (*aSeqList).end()-1, (*aSeqList).end(), SeqGreater );
		}
		(*aSeqTree).Size++;
	}
}

void TreeNode::Print( char * PrefixString, FILE * aFile )
{
  NodeVector::iterator Itrtr;

	if( ItemIsIntra )
		fprintf( aFile, "%sNode = _%d   Support = %d   ItemsetNumber = %d\n", PrefixString, Item, Support, ItemsetNumber );
	else
		fprintf( aFile, "%sNode =  %d   Support = %d   ItemsetNumber = %d\n", PrefixString, Item, Support, ItemsetNumber );

	if( Children != NULL && NumOfChildren() > 0 )
	{
		char tmpString[256] = "   ";

		fprintf( aFile, "  %sChildren\n", PrefixString );

		strncat( tmpString, PrefixString, 250 );
		//strcat( tmpString, "  " );

		for( Itrtr = (*Children).begin(); Itrtr != (*Children).end(); Itrtr++ )
		{
			//fprintf( aFile, "    " );
			(*(*Itrtr)).Print( tmpString, aFile );
		}
	}

	//fprintf( aFile, "\n" );
}

TreeNode::~TreeNode()
{
	NodeVector::iterator Itrtr;

	if( Children )
		for( Itrtr = (*Children).begin(); Itrtr != (*Children).end(); Itrtr++)
			delete *Itrtr;
}

//************************
//************************

//************************
//************************


bool HdrTreeNodeLess( TreeNode * a, TreeNode * b )
{
	return a > b;
}

int HdrTreeNodeCompare( TreeNode ** a, TreeNode ** b )
{
	if( *a < *b )
		return 1;
	else if( *a > *b )
		return -1;
	else   
		return 0;
}


SequenceList::SequenceList( int ItemsCount )
{
	PeakNumOfTreeNodes = 0;
	NumOfTreeNodes = 0;

	Root = new TreeNode();
	IncNumOfTreeNodes();

	NumOfItems = ItemsCount;
	Header = new NodeVector[ NumOfItems ];

	NumOfAdds = 0;
	NumOfDels = 0;
	NumOfAddIfClosed = 0;
	NumIsClosedSubsetOfBranch = 0;
	NumIsClosedSupersetOfBranch = 0;
}


inline void SequenceList::IncNumOfTreeNodes()
{
	NumOfTreeNodes++;
	if( NumOfTreeNodes > PeakNumOfTreeNodes )
		PeakNumOfTreeNodes = NumOfTreeNodes;
}

inline void SequenceList::DecNumOfTreeNodes()
{
	NumOfTreeNodes--;
}

// Returns the item in the last itemset of aSeq, with shortest header list.
int SequenceList::LeastFreqInHeader( Sequence * aSeq )
{
	int * ItemPtr;
	int Res = -1;
	int Min = 10000000;
	int HeaderSize;

	for( ItemPtr=(*aSeq).StartPtr+(*aSeq).Len-1; *ItemPtr!=-1 && ItemPtr>=(*aSeq).StartPtr; ItemPtr-- )
	{
		HeaderSize = Header[*ItemPtr].size();
		if( HeaderSize < Min )
		{
			Min = HeaderSize;
			Res = *ItemPtr;
		}
		if( Min==0 ) break;
	}

	return Res;
}

// Returns true, if aSeq is a subset of the sequence in the tree with aNode as its last Item.
bool SequenceList::IsClosedSubsetOfBranch( Sequence * aSeq, TreeNode * aNode )
{
	int * ItemPtr;
	int * EndOfItemset;
	int NumOfRemItemsets;
	TreeNode * CurNode = aNode;


	NumIsClosedSubsetOfBranch++;

	NumOfRemItemsets = (*aSeq).NumOfItemSets();

	if( (*CurNode).ItemsetNumber < NumOfRemItemsets || (*aSeq).Support > (*CurNode).Support )
		return false;

	ItemPtr = (*aSeq).StartPtr+(*aSeq).Len-1;
	EndOfItemset = ItemPtr;

	// For all itemsets in the sequence.
	while( ItemPtr >= (*aSeq).StartPtr )
	{

		// For this itemset.
		while( true )
		{
			if( (*CurNode).Item == *ItemPtr )
			{
				ItemPtr--;

				if( ItemPtr < (*aSeq).StartPtr )
					return true;

				// Check previous itemset in the sequence.
				if( *ItemPtr == -1 )
				{
					ItemPtr--;
					break;
				}

			}

			if( !(*CurNode).ItemIsIntra )
			{
				if( (*CurNode).ItemsetNumber <= NumOfRemItemsets )
					return false;

				ItemPtr = EndOfItemset;
			}

			CurNode = (*CurNode).Parent;
		}

		NumOfRemItemsets--;
		//Move to the end of the previous itemset.
		while( (*CurNode).ItemIsIntra && (*CurNode).Parent!=NULL )
			CurNode = (*CurNode).Parent;
		CurNode = (*CurNode).Parent;


		EndOfItemset = ItemPtr;

	}
	return false;
}

// Returns true, if aSeq is a superset of the sequence in the tree with aNode as its last element.
bool SequenceList::IsClosedSupersetOfBranch( Sequence * aSeq, TreeNode * aNode )
{
	int * ItemPtr;
	TreeNode * EndOfItemset;
	int NumOfRemItemsets;
	TreeNode * CurNode = aNode;

	NumIsClosedSupersetOfBranch++;

	NumOfRemItemsets = (*aSeq).NumOfItemSets();

	if( (*CurNode).ItemsetNumber > NumOfRemItemsets || (*CurNode).Support > (*aSeq).Support )
		return false;

	ItemPtr = (*aSeq).StartPtr+(*aSeq).Len-1;
	EndOfItemset = aNode;

	while( ItemPtr >= (*aSeq).StartPtr )
	{
		// For this itemset.
		//while( true )
		while( ItemPtr >= (*aSeq).StartPtr )
		{
			if( (*CurNode).Item == *ItemPtr )
			{
				//if( (*(*CurNode).Parent).ItemsetNumber == 0 )
				if( (*(*CurNode).Parent).ItemsetNumber <= 0 )
					return true;

				// Check previous itemset in the sequence.
				if( !(*CurNode).ItemIsIntra )
				{
					CurNode = (*CurNode).Parent;
					break;
				}
				CurNode = (*CurNode).Parent;
			}

			if( *ItemPtr == -1 )
			{
				NumOfRemItemsets--;
				if( (*CurNode).ItemsetNumber > NumOfRemItemsets )
					return false;

				CurNode = EndOfItemset;
			}
			ItemPtr--;
		}

		//Move to the end of the previous itemset.
		while( *ItemPtr!=-1 && ItemPtr >= (*aSeq).StartPtr )
			ItemPtr--;
		ItemPtr--;


		EndOfItemset = CurNode;

	}
	return false;
}

bool SequenceList::IsContained( Sequence * aSeq )
{
	int NumOfItemSets;
	int anItem;
  NodeVector::iterator Itrtr;

	if( (*aSeq).Len == 0 )
		return true;


	// Just check for the item in the item list with shortest header list.
	//anItem = LeastFreqInHeader( aSeq );
	anItem = *((*aSeq).StartPtr+(*aSeq).Len-1); // Last Item of the sequence.

	if( Header[anItem].size() == 0 )
		return false; 

	NumOfItemSets = (*aSeq).NumOfItemSets();

	for( Itrtr = Header[anItem].begin(); Itrtr != Header[anItem].end(); Itrtr++ )
	{

		if( (*(*Itrtr)).ItemsetNumber < NumOfItemSets || (*(*Itrtr)).Support < (*aSeq).Support )
			continue;
		
		if( IsClosedSubsetOfBranch( aSeq, (*Itrtr) ) )
			return true;
	}

	return false;
}

void SequenceList::RemoveFromHeaderList( TreeNode * aNode )
{
  NodeVector::iterator Itrtr;

	Itrtr = FindInHeaderList( aNode ) ;
	while( Itrtr!=NULL )
	{
		Header[(*aNode).Item].erase( Itrtr );
		Itrtr = FindInHeaderList( aNode ) ;
	}

/*
	for( Itrtr = Header[(*aNode).Item].begin(); Itrtr<Header[(*aNode).Item].end(); Itrtr++ )
	{
		if( (*Itrtr) == aNode )
			Itrtr = Header[(*aNode).Item].erase( Itrtr );
	}
*/
}

void SequenceList::DeleteSequence( TreeNode * aNode )
{
	TreeNode * tmpNode;
	TreeNode * tmpChildNode;
	int aSup;
	int NewSup;

	aSup = (*aNode).Support;
	tmpNode = aNode;

	while( (*tmpNode).Support==aSup && (*tmpNode).Parent!=NULL && (*tmpNode).NumOfChildren() == 0 )
	{
		RemoveFromHeaderList( tmpNode );

		tmpChildNode = tmpNode;
		tmpNode = (*tmpNode).Parent;
		(*tmpNode).DelChild( tmpChildNode );

		delete tmpChildNode;
		DecNumOfTreeNodes();
	}

	if( (*tmpNode).Support==aSup && (*tmpNode).Parent!=NULL )
	{
		NewSup = (*tmpNode).MaxChildSupport();
		(*tmpNode).Support = NewSup;
		tmpNode = (*tmpNode).Parent;
	}

	while( (*tmpNode).Support==aSup && (*tmpNode).Parent!=NULL )
	{
		if( (*tmpNode).Support > (*tmpNode).MaxChildSupport() )
			(*tmpNode).Support = NewSup;
		tmpNode = (*tmpNode).Parent;
	}

}


void SequenceList::DeleteClosedSubsets( Sequence * aSeq )
{
  NodeVector::iterator Itrtr;
  NodeVector::iterator tmpItrtr;
	int NumOfItemSets;
	int * ItemPtr;

	if( (*aSeq).Len == 0 )
		return;

	NumOfItemSets = (*aSeq).NumOfItemSets();

	// for each itemset in the sequence.
	for( ItemPtr=(*aSeq).StartPtr+(*aSeq).Len-1; ItemPtr>=(*aSeq).StartPtr; ItemPtr-- )
	{
		// for each item in an itemset.
		for( ; *ItemPtr!=-1 && ItemPtr>=(*aSeq).StartPtr; ItemPtr-- )
		{

			if( Header[*ItemPtr].size() > 0 )
			{
				Itrtr = Header[*ItemPtr].end() - 1;
				while( Itrtr >= Header[*ItemPtr].begin()  )
				{
					//try 
					{
						if( (*(*Itrtr)).ItemsetNumber > NumOfItemSets || !(*(*Itrtr)).LastItemOfSequence() ) 
						{
							Itrtr--;
							continue;
						}
						
						if( IsClosedSupersetOfBranch( aSeq, (*Itrtr) ) )
						{
							tmpItrtr = Itrtr;
							tmpItrtr--;

							NumOfDels++;
							DeleteSequence( (*Itrtr) );
							Itrtr = tmpItrtr;
						} else 
							Itrtr--;
					} /*catch(...)	{
						Itrtr--;
						printf( " Error in Delete Subsets in SequenceList.\n" );
					}*/
				}
			}

		}
	}

}

inline NodeVector::iterator SequenceList::FindInHeaderList( TreeNode * aNode )
{

  NodeVector::iterator Itrtr;

	Itrtr = (TreeNode **) bsearch( &aNode, Header[(*aNode).Item].begin(), Header[(*aNode).Item].size(), sizeof( TreeNode *), (int (*)(const void*, const void*))HdrTreeNodeCompare );

	return Itrtr;

/*
  NodeVector::iterator Itrtr;
  NodeVector::iterator ResultItrtr = NULL;

	for( Itrtr = Header[(*aNode).Item].begin(); Itrtr<Header[(*aNode).Item].end(); Itrtr++ )
	{
		if( (*Itrtr) == aNode )
		{
			ResultItrtr = Itrtr;
			break;
		}
	}
	return ResultItrtr;
*/
}

// Updates the Header list for the sequences ending with LastNode.
inline void SequenceList::UpdateHeaderList( TreeNode * LastNode )
{
  NodeVector::iterator Itrtr;
	TreeNode * CurNode;
	int LastSup = 0;


	// Add the last instance of each item in the sequence to the header list.
	for( CurNode=LastNode; (*CurNode).Parent!=NULL; CurNode=(*CurNode).Parent )
	{

		if( LastSup != (*CurNode).Support )
		{
			// inter_freq_idx is used to avoid adding duplicated items to the header list.
			memset( inter_freq_idx, 0, sizeof(int)*gN_ITEMS );

			LastSup = (*CurNode).Support;
		}

		if( inter_freq_idx[ (*CurNode).Item ] == 0 )
		{
			Itrtr = FindInHeaderList( CurNode );

			if( Itrtr == NULL )
			{
				Header[(*CurNode).Item].push_back( CurNode );

				// To keep the Header lists sorted based on the ItemsetNumber.
				inplace_merge( Header[(*CurNode).Item].begin(), Header[(*CurNode).Item].end()-1, Header[(*CurNode).Item].end(), HdrTreeNodeLess ); 
			}
	
			inter_freq_idx[ (*CurNode).Item ] = 1;

		}
	}
}

int SequenceList::AddIfClosedSeq( Sequence * aSeq )
{
	TreeNode * tmpNode;
	TreeNode * CurNode;
	TreeNode * LastOldNode = NULL;
	int * CurItem;
	bool Intra = false;

	NumOfAddIfClosed ++;


	// aSeq was not added.
	if( IsContained( aSeq ) ) 
	{
		delete aSeq;
		return 1;
	}

	DeleteClosedSubsets( aSeq );

	
	CurNode = Root;

	// Add the sequence to the tree.
	for( CurItem = (*aSeq).StartPtr; !(*aSeq).EndOfSequence( CurItem ); CurItem++ )
	{
		for( ; !(*aSeq).EndOfItemSet( CurItem ); CurItem++ )
		{
			tmpNode = new TreeNode( *CurItem, Intra, (*aSeq).Support );
			CurNode = (*CurNode).AddChild( tmpNode );
			if( CurNode == tmpNode ) 
				IncNumOfTreeNodes();


			// Keep the parent of the first new added node to the tree.
			if( LastOldNode==NULL && CurNode==tmpNode )
				LastOldNode = (*CurNode).Parent;

			Intra = true;
		}
		Intra = false;
	}


	UpdateHeaderList( CurNode );
	
	NumOfAdds++;

	delete aSeq;
	
	// aSeq was added.
	return 0;
}

void SequenceList::InternalPrintRules( FILE * aFile )
{

  NodeVector::iterator Itrtr;
	SeqList * SortedSeqList = NULL;

	#if defined( _SORT_RESULTS )
		SeqList::iterator ListItrtr;
		SortedSeqList = new SeqList;
	#endif

	if( Root )
		if( (*Root).Children != NULL )
		{
			for( Itrtr = (*(*Root).Children).begin(); Itrtr != (*(*Root).Children).end(); Itrtr++ )
			{
				(*(*Itrtr)).PrintRules( aFile, buf_idx, 0, this, SortedSeqList, 1 ); // buf_idx is a buffer defined globaly.
			}
		}

	#if defined( _SORT_RESULTS )
		fprintf( aFile, "\n" );
		for( ListItrtr=(*SortedSeqList).begin(); ListItrtr<(*SortedSeqList).end(); ListItrtr++ )
		{
			(*(*ListItrtr)).Print( aFile );
			delete (*ListItrtr);
		}
		delete SortedSeqList;
	#endif
}

void SequenceList::Print( FILE * aFile )
{
	Size = 0;
	InternalPrintRules( aFile );

	fprintf( aFile, "\n" );
	fprintf( aFile, "# of elements added %d\n", NumOfAdds );
	fprintf( aFile, "# of elements deleteded %d\n", NumOfDels );
	fprintf( aFile, "# of times AddIfClosedSeq called %d\n", NumOfAddIfClosed );
	fprintf( aFile, "# of times IsClosedSubsetOf called %d\n", NumIsClosedSubsetOfBranch + NumIsClosedSupersetOfBranch );
	fprintf( aFile, "# of elements in the list %d\n", Size );
	fprintf( aFile, "# of nodes in the final tree %d\n", NumOfTreeNodes );
	fprintf( aFile, "Peak # of nodes used in tree %d\n", PeakNumOfTreeNodes );

//	fprintf( aFile, "\n\n**************************************\n\n", Size );
//	PrintTree( aFile );
}

void SequenceList::PrintTree( FILE * aFile )
{
  NodeVector::iterator Itrtr;

	if( Root )
		(*Root).Print( "", aFile );

	fprintf( aFile, "\n Header Table:\n" );

	for( int i=0; i<NumOfItems; i++ )
	{

		fprintf( aFile, "  List for %d (Len=%d) => ", i, Header[i].size() );
		for( Itrtr = Header[i].begin(); Itrtr != Header[i].end(); Itrtr++)
		{
			fprintf( aFile, "  %d(%d)",  (*(*Itrtr)).Item, (*(*Itrtr)).Support );
		}
		fprintf( aFile, " \n" );
	}

}


SequenceList::~SequenceList()
{
	delete[] Header;
	delete Root;
}
//************************************
//************************************
//************************************

//************************
//************************
#endif // _NEW_SEQUENCE_LIST < 20
//************************
//************************
#else // defined( _NEW_SEQUENCE_LIST )
//************************
//************************

SequenceList::SequenceList()
{
	NumOfAdds = 0;
	NumOfDels = 0;
	NumOfAddIfClosed = 0;
	NumIsClosedSubsetOf = 0;
}

inline void SequenceList::AddSeq( Sequence * aSeq )
{
			//List.push_back( aSeq );
			// To keep the List vector sorted.
			//inplace_merge( List.begin(), List.end()-1, List.end(), SeqGreater );
	List.insert( List.end(), aSeq );
	NumOfAdds++;
}

inline SeqList::iterator SequenceList::Del( SeqList::iterator anItr )
{
	NumOfDels++;
	return List.erase( anItr );
}

// Tries to add aSeq to the list if non of the list elements
// is a closed superset of aSeq.
// Sequences in the list that are closed subset of aSeq
// will be deleted from the list.
// Returns true if aSeq is added.
bool SequenceList::AddIfClosedSeq( Sequence * aSeq )
{
	SeqList::iterator Itrtr;
	bool IsSuper = false;


	NumOfAddIfClosed++;
	for( Itrtr = List.begin(); Itrtr != List.end(); )
	{
		if( !IsSuper )
		{
			NumIsClosedSubsetOf++;
			if( (*aSeq).IsClosedSubsetOf( (*Itrtr) ) )
				return false;
		}

		NumIsClosedSubsetOf++;
		if( (*(*Itrtr)).IsClosedSubsetOf( aSeq ) )
		{

			delete (*Itrtr) ;

			IsSuper = true;
			Itrtr = Del( Itrtr );
		} else
			Itrtr++;
	}

	AddSeq( aSeq );
	return true;
} 

void SequenceList::Print( FILE *aFile )
{

	SeqList::iterator Itrtr;

#if defined( _SORT_RESULTS )
  SeqList::iterator ListItrtr;
	SeqList * SortedSeqList = NULL;
	SortedSeqList = new SeqList;
#endif

	fprintf( aFile, "\n" );

	for( Itrtr = List.begin(); Itrtr != List.end(); Itrtr++)
	{
		#if defined( _SORT_RESULTS )
			(*SortedSeqList).push_back( (*Itrtr) );
			// To keep the List vector sorted.
			inplace_merge( (*SortedSeqList).begin(), (*SortedSeqList).end()-1, (*SortedSeqList).end(), SeqGreater );
		#else
			(*(*Itrtr)).Print( aFile );
		#endif
	}

#if defined( _SORT_RESULTS )
	fprintf( aFile, "\n" );
	for( ListItrtr=(*SortedSeqList).begin(); ListItrtr<(*SortedSeqList).end(); ListItrtr++ )
	{
		(*(*ListItrtr)).Print( aFile );
	}
	delete SortedSeqList;
#endif

	fprintf( aFile, "\n" );
	fprintf( aFile, "# of elements added %d\n", NumOfAdds );
	fprintf( aFile, "# of elements deleteded %d\n", NumOfDels );
	fprintf( aFile, "# of times AddIfClosedSeq called %d\n", NumOfAddIfClosed );
	fprintf( aFile, "# of times IsClosedSubsetOf called %d\n", NumIsClosedSubsetOf );
	fprintf( aFile, "# of elements in the list %d\n", List.size() );

}

SequenceList::~SequenceList()
{
	SeqList::iterator Itrtr;

	for( Itrtr = List.begin(); Itrtr != List.end(); Itrtr++)
		delete *Itrtr;
}

#endif // defined( _NEW_SEQUENCE_LIST )
//************************
//************************

//************************
//************************

//************************
//************************

//************************
//************************


void RTATest( const struct PROJ_DB * pDB )
{
/*
	int tmp = 0;
	int aMat[10];
	SequenceList * aSeqTree;
	Sequence * aSeq;


	aSeqTree = new SequenceList( 10 );


	aMat[0] = 1;
	aMat[1] = -1;
	aMat[2] = 2;
	aMat[3] = 3;
	aSeq = new Sequence( aMat, 4, 4 );
	(*aSeq).Print();
	printf( " NumOfItemSets = %d\n", (*aSeq).NumOfItemSets() );
	if( (*aSeqTree).AddIfClosedSeq( aSeq ) == 0 )
	{
		printf( "was added to the tree.\n\n" );
	} else {
		printf( "was NOT added to the tree.\n\n" );
	}

	aMat[0] = 1;
	aMat[1] = -1;
	aMat[2] = 2;
	aSeq = new Sequence( aMat, 3, 6 );
	(*aSeq).Print();
	printf( " NumOfItemSets = %d\n", (*aSeq).NumOfItemSets() );
	if( (*aSeqTree).AddIfClosedSeq( aSeq ) == 0 )
	{
		printf( "was added to the tree.\n\n" );
	} else {
		printf( "was NOT added to the tree.\n\n" );
	}

	aMat[0] = 1;
	aMat[1] = -1;
	aMat[2] = 2;
	aMat[3] = 4;
	aSeq = new Sequence( aMat, 4, 4 );
	(*aSeq).Print();
	printf( " NumOfItemSets = %d\n", (*aSeq).NumOfItemSets() );
	if( (*aSeqTree).AddIfClosedSeq( aSeq ) == 0 )
	{
		printf( "was added to the tree.\n\n" );
	} else {
		printf( "was NOT added to the tree.\n\n" );
	}

	aMat[0] = 1;
	aMat[1] = -1;
	aMat[2] = 2;
	aMat[3] = 4;
	aMat[4] = 5;
	aSeq = new Sequence( aMat, 5, 2 );
	(*aSeq).Print();
	printf( " NumOfItemSets = %d\n", (*aSeq).NumOfItemSets() );
	if( (*aSeqTree).AddIfClosedSeq( aSeq ) == 0 )
	{
		printf( "was added to the tree.\n\n" );
	} else {
		printf( "was NOT added to the tree.\n\n" );
	}

	aMat[0] = 1;
	aMat[1] = -1;
	aMat[2] = 2;
	aMat[3] = 4;
	aMat[4] = 6;
	aSeq = new Sequence( aMat, 5, 2 );
	(*aSeq).Print();
	printf( " NumOfItemSets = %d\n", (*aSeq).NumOfItemSets() );
	if( (*aSeqTree).AddIfClosedSeq( aSeq ) == 0 )
	{
		printf( "was added to the tree.\n\n" );
	} else {
		printf( "was NOT added to the tree.\n\n" );
	}

	aMat[0] = 1;
	aMat[1] = -1;
	aMat[2] = 2;
	aMat[3] = 4;
	aMat[4] = 7;
	aSeq = new Sequence( aMat, 5, 5 );
	(*aSeq).Print();
	printf( " NumOfItemSets = %d\n", (*aSeq).NumOfItemSets() );
	if( (*aSeqTree).AddIfClosedSeq( aSeq ) == 0 )
	{
		printf( "was added to the tree.\n\n" );
	} else {
		printf( "was NOT added to the tree.\n\n" );
	}

	aMat[0] = 1;
	aMat[1] = -1;
	aMat[2] = 2;
	aMat[3] = 4;
	aMat[4] = 6;
	aMat[5] = 8;
	aSeq = new Sequence( aMat, 6, 6 );
	(*aSeq).Print();
	printf( " NumOfItemSets = %d\n", (*aSeq).NumOfItemSets() );
	if( (*aSeqTree).AddIfClosedSeq( aSeq ) == 0 )
	{
		printf( "was added to the tree.\n\n" );
	} else {
		printf( "was NOT added to the tree.\n\n" );
	}
	buf_idx=(int*)malloc(sizeof(int)*1000);


	//(*aSeqTree).PrintTree();
	(*aSeqTree).Print();

	free( buf_idx );
	delete aSeqTree;
	exit( 0 );

*/

/*	int * RecStart;
	//SeqWrap * aWrap;
	Prefix * aWrap;
	Sequence * aSeq;
	FILE *File1 = NULL;
	FILE *File2 = NULL;

  File1 = file_open( "PrjDBNewStruc.txt", "w" ); 
  File2 = file_open( "PrjDBOldStruc.txt", "w" ); 

	for( int i=0; i<(*pDB).m_nSup; i++ )
	{
		RecStart = GetStartPtr( pDB, i );
		//aWrap = new SeqWrap( RecStart );
		aWrap = new Prefix( RecStart, GetEndPtr( RecStart ) );
		(*aWrap).GetFirst();
		(*aWrap).Print( File1 );
		delete aWrap;

		aSeq = new Sequence( RecStart, true );
		(*aSeq).Print( File2 );
		delete aSeq;
	}

  fclose( File1 );
  fclose( File2 );
*/
}

#endif // !defined( _FIND_MAX_SEQS ) && !defined( _FIND_FREQUENT_SEQS )
