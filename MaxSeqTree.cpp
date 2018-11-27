#include "../Global.h"
#include "../ProjDB.h"
#include "SeqTree.h"

#if defined( _FIND_MAX_SEQS )

// Assumes itemsets have the same size.
inline bool ItemSetGreater( ItemSet * a, ItemSet * b )
{
	if( (*a).NumOfItems() > (*b).NumOfItems() )
		return true;
	else if( (*a).NumOfItems() < (*b).NumOfItems() )
		return false;

	for( int i=0; i<(*a).NumOfItems(); i++ )
	{
		if( (*a).Items[i] > (*b).Items[i] )
			return true;
		if( (*a).Items[i] < (*b).Items[i] )
			return false;
	}
	return false;
}

inline bool SeqGreater( Sequence * a, Sequence * b )
{
	ElementVector::iterator aItrtr;
	ElementVector::iterator bItrtr;

	if( (*a).Sup > (*b).Sup )
		return true;
	else if( (*a).Sup < (*b).Sup )
		return false;
	else if( (*a).Elements.size() > (*b).Elements.size() )
		return true;
	else if( (*a).Elements.size() < (*b).Elements.size() )
		return false;

	bItrtr = (*b).Elements.begin();
	for( aItrtr = (*a).Elements.begin(); aItrtr != (*a).Elements.end(); aItrtr++)
	{
		if( ItemSetGreater( *aItrtr, *bItrtr ) )
			return true;
		if( ItemSetGreater( *bItrtr, *aItrtr ) )
			return false;
		bItrtr++;
	}
	return false;
}

//************************
//************************

ItemSet::ItemSet( int NumItems, int * Itms )
{
	int i;

	Count = NumItems;
	Items = new int[Count];

	if( Itms )
		for( i=0; i<Count; i++ )
		{
			Items[i]=Itms[i];
		}

}

ItemSet::ItemSet( ItemSet * anItemSet )
{
	int i;

	Count = (*anItemSet).NumOfItems();
	Items = new int[Count];

	if( (*anItemSet).Items )
		for( i=0; i<Count; i++ )
		{
			Items[i]=(*anItemSet).Items[i];
		}
}

inline bool ItemSet::IsSubsetOf( ItemSet * anItemSet )
{
	int j;
	int MyCnt;
	int OtherCnt;

	MyCnt = NumOfItems();
	OtherCnt = (*anItemSet).NumOfItems();

	if( MyCnt > OtherCnt )
		return false;

	j = 0;
	for( int i=0; i<MyCnt; i++ )
	{
		while( Items[i] != (*anItemSet).Items[j] )
		{
			j++;
			if( OtherCnt-j < MyCnt-i ) 
				return false;
		}
	}

	return true;
}

inline bool ItemSet::IsLessThan( ItemSet * anItemSet )
{
	int MyCnt;
	int OtherCnt;

	MyCnt = NumOfItems();
	OtherCnt = (*anItemSet).NumOfItems();

	if( MyCnt < OtherCnt )
		return true;

	if( MyCnt > OtherCnt )
		return false;

	// MyCnt == OtherCnt
	for( int i=0; i<MyCnt; i++ )
	{
		if( Items[i] < (*anItemSet).Items[i] )
			return true;
		if( Items[i] > (*anItemSet).Items[i] )
			return false;
	}

	return true;
}

inline int ItemSet::Compare( ItemSet * anItemSet )
{
	int MyCnt;
	int OtherCnt;

	MyCnt = NumOfItems();
	OtherCnt = (*anItemSet).NumOfItems();

	if( MyCnt < OtherCnt )
		return -1;

	if( MyCnt > OtherCnt )
		return +1;

	// MyCnt == OtherCnt
	for( int i=0; i<MyCnt; i++ )
	{
		if( Items[i] < (*anItemSet).Items[i] )
			return -1;
		if( Items[i] > (*anItemSet).Items[i] )
			return +1;
	}

	return 0;
}

void ItemSet::Print1( FILE *aFile )
{
	int i;

	fprintf( aFile, " (" );
	for( i=0; i<Count; i++ )
	{
		fprintf( aFile, " %d ", Items[i] );
	}
	fprintf( aFile, ") " );

}

void ItemSet::Print( FILE *aFile )
{
	int i;

	fprintf( aFile, " (" ); 
	for( i=0; i<Count; i++ ) 
	{
		fprintf( aFile, " %d ", Items[i] );
	}
	fprintf( aFile, ") " );

}

void ItemSet::Add ( int Item )
{
	int i;
	int * tmp;

	tmp = new int[Count+1];


	for( i=0; i<Count; i++ )
	{
		tmp[i] = Items[i];
	}
	tmp[Count] = Item;
	delete[] Items;
	Items = tmp;

	Count++;
}

ItemSet::~ItemSet()
{
	delete[] Items;
}

//************************
//************************

//************************
//************************

Sequence::Sequence()
{
	Sup = 0;
}

Sequence::Sequence( Sequence * aSeq )
{
  ElementVector::iterator Itrtr;

	Sup = (*aSeq).Sup;

	for( Itrtr = (*aSeq).Elements.begin(); Itrtr != (*aSeq).Elements.end(); Itrtr++)
		Add( new ItemSet( (*Itrtr) ) );

}

Sequence::Sequence( int * Ptr, int Support )
{
	ItemSet * anItemSet;
	int Start;

	Sup = Support;

	Start = 0;

  int * dataset = Ptr;
	int * StartPtr = Ptr;

	int i = 0;

  for (; *dataset!=-2; dataset++)
	{
		if( *dataset == -1 )
		{
			anItemSet = new ItemSet( i-Start, StartPtr );
			StartPtr = dataset + 1;
			Start = i+1;
			Add( anItemSet );
		}
		i++;
	}
	//Print();
	//printf( "\n" );
}

Sequence::Sequence( int * Pat, int PatLen, int Support )
{
	ItemSet * anItemSet;
	int Start;

	Sup = Support;

	Start = 0;

	if( PatLen > 1 )
	{
		for( int i=0; i<PatLen; i++)
		{
			if( *(Pat + i) == -1 )
			{
				anItemSet = new ItemSet( i-Start, Pat+Start );
				Start = i+1;
				Add( anItemSet );
			}
		}
		anItemSet = new ItemSet( i-Start, Pat+Start );
		Add( anItemSet );
	} else {
		int tmp[1];
		tmp[0] = *Pat;
		anItemSet = new ItemSet( 1, tmp );
		Add( anItemSet );
	}
}

Sequence::Sequence( const struct PROJ_DB *proj_db, int Support )
{
	ItemSet * anItemSet;
	int Start;

	Sup = Support;

	Start = 0;

	if( proj_db[0].m_nPatLen > 1 )
	{
		for( int i=0; i<proj_db[0].m_nPatLen; i++)
		{
			if( proj_db[0].m_pnPat[i] == -1 )
			{
				anItemSet = new ItemSet( i-Start, &proj_db[0].m_pnPat[Start] );
				Start = i+1;
				Add( anItemSet );
			}
		}
		anItemSet = new ItemSet( i-Start, &proj_db[0].m_pnPat[Start] );
		Add( anItemSet );
	} else {
		int tmp[1];
		tmp[0] = int( proj_db[0].m_pnPat );
		anItemSet = new ItemSet( 1, tmp );
		Add( anItemSet );
	}

//	Print();
//	printf( "\n" );
}

Sequence::Sequence( const struct PROJ_DB *proj_db, int * aSeqPtr ) //: Sequence( proj_db, 0 )
{
#if defined( _PERFORM_COMMON_PREFIX_CHECKING )
	int * aPtr;
	int i;
	SeqWrap * aSeq;

	if( (*proj_db).m_nPatLen > 1 )
	{
		for( int j=0; j<(*proj_db).m_nPatLen; j++ )
			buf_idx[j] = (*proj_db).m_pnPat[j];
	} else
		buf_idx[0] = int( (*proj_db).m_pnPat );

	i = (*proj_db).m_nPatLen;
	aPtr = aSeqPtr;

	aSeq = new SeqWrap( aSeqPtr );

	aPtr = (*aSeq).GetFirst();

	while( *aPtr != -2 )
	{
		buf_idx[i++] = *aPtr;
		aPtr = (*aSeq).GetNext();
	}

	delete aSeq;

	int PatLen = i - 1;
	int * Pat = buf_idx;

/////////////////
	ItemSet * anItemSet;
	int Start;

	Sup = 0;

	Start = 0;

	if( PatLen > 1 )
	{
		for( int i=0; i<PatLen; i++)
		{
			if( *(Pat + i) == -1 )
			{
				anItemSet = new ItemSet( i-Start, Pat+Start );
				Start = i+1;
				Add( anItemSet );
			}
		}
		anItemSet = new ItemSet( i-Start, Pat+Start );
		Add( anItemSet );
	} else {
		int tmp[1];
		tmp[0] = *Pat;
		anItemSet = new ItemSet( 1, tmp );
		Add( anItemSet );
	}

/////////////////


#endif // defined( _PERFORM_COMMON_PREFIX_CHECKING )
}


#if !defined( _USE_MAX_TREE )
// Returns true if this sequence is the Max subset of aSeq.
bool Sequence::IsMaxSubsetOf( Sequence * aSeq )
{
  ElementVector::iterator aSeqPtr;
  ElementVector::iterator thisPtr;

	int aSeqLen = (*aSeq).Elements.size();
	int thisLen = Elements.size();

	if( Elements.size() == 0 )
		return true;

	thisPtr = Elements.begin();
	aSeqPtr = (*aSeq).Elements.begin();

	if( aSeqLen >= thisLen && aSeqLen > 0 )
	{
		while( aSeqPtr != (*aSeq).Elements.end() )
		{
			if( (*(*thisPtr)).IsSubsetOf( (*aSeqPtr) ) )
				thisPtr++;
			if( thisPtr == Elements.end() )
				return true;

			aSeqPtr++;
		}
	}

	return false;

}

#endif // !defined( _USE_MAX_TREE )

void Sequence::Add( struct PROJ_SEQ * PrjSeq )
{
	ItemSet * anItemSet;
	int * Start;
	int * intPtr;

	MoveLast();
	anItemSet = Current();

	if( (*PrjSeq).m_nProjCount > 1 )
		intPtr = (*PrjSeq).m_ppSeq[0];
	else
		intPtr = (int *)(*PrjSeq).m_ppSeq;

	// Should be improvemnet. 123
	while( *intPtr != -1 && *intPtr != -2 )
	{
		(*anItemSet).Add( *intPtr );
		intPtr ++;
	}

	while( *intPtr != -2 )
	{
		Start = intPtr;
		while( *intPtr != -1 )
		{
 
			intPtr ++;
		}
		if( Start != intPtr )
		{
			anItemSet = new ItemSet( intPtr-Start, Start );
			Add( anItemSet );
		}
		intPtr ++;
	}
}


void Sequence::Print( FILE *aFile )
{
  ElementVector::iterator Itrtr;

	fprintf( aFile, " <" );

	for( Itrtr = Elements.begin(); Itrtr != Elements.end(); Itrtr++)
		(*(*Itrtr)).Print( aFile );

	fprintf( aFile, "> Support = %d\n", Sup );

}

Sequence::~Sequence()
{
  ElementVector::iterator Itrtr;

	for( Itrtr = Elements.begin(); Itrtr != Elements.end(); Itrtr++)
		delete *Itrtr;
}

//************************
//************************

//************************
//************************

//************************
//************************
#if defined( _USE_MAX_TREE )
//************************
//************************
SeqTree::SeqTree( int ItemsCount )
{
	Root = new TreeNode();
	NumOfItems = ItemsCount;
	Header = new NodeVector[NumOfItems+1];
	//Header = new NodeList[NumOfItems+1];

	NumOfAdds = 0;
	NumOfDels = 0;
	NumOfAddIfClosed = 0;
	NumIsMaxSubsetOf = 0;

}

bool TreeNodeLevelLess( TreeNode * a, TreeNode * b )
{
	return (*a).Level > (*b).Level;
}

int TreeNodeLevelCompare( TreeNode ** a, TreeNode ** b )
{
	if( (*(*a)).Level < (*(*b)).Level )
		return 1;
	else if( (*(*a)).Level > (*(*b)).Level )
		return -1;
	else   
		return 0;
	//return (*(*(*a)).Item).Compare( (*(*b)).Item );
}

void SeqTree::RemoveFromHeaderList( TreeNode * aNode )
{
	ItemSet * anItemSet;
	int anItem;
	TreeNode ** Res;
  NodeVector::iterator Itrtr;

	anItemSet = (*aNode).Item;
	// Delete the links from the header table
	for( int i=0; i<(*anItemSet).NumOfItems(); i++ )
	{
		anItem = (*anItemSet).Items[i];

		Res = (TreeNode **) bsearch( &aNode, Header[anItem].begin(), Header[anItem].size(), sizeof( TreeNode *), (int (*)(const void*, const void*))TreeNodeLevelCompare );

		if( !Res ) continue;

		Itrtr = Res;
		
		while( (*(*Itrtr)).Level == (*aNode).Level && Itrtr != Header[anItem].begin() ) 
			Itrtr--;
		
		while( (*(*Itrtr)).Level >= (*aNode).Level && Itrtr != Header[anItem].end() ) 
		{
			if( (*Itrtr) == aNode )
				Itrtr = Header[anItem].erase( Itrtr );
			else
				 Itrtr++;
		}
	}
	
}

void SeqTree::DeleteSequence( TreeNode * aNode )
{
	TreeNode * Parent;


	if( (*aNode).NumOfChildren() == 0 && (*aNode).Parent )
	{
		RemoveFromHeaderList( aNode );

		Parent = (*aNode).Parent;

		//if( (*Parent).NumOfChildren() > 1 )
			(*Parent).DelChild( aNode );

		//DeleteSequence( (*aNode).Parent );
		DeleteSequence( Parent );

		delete aNode;
	}

}

// If aSeq is a subset of the sequence in the tree with aNode as its last element.
bool SeqTree::IsSubsetOfBranch( Sequence * aSeq, TreeNode * aNode )
{
	int NumOfRemElmnts;
	ItemSet * anItemSet;
	TreeNode * CurNode;

	(*aSeq).MoveLast();
	anItemSet = (*aSeq).Current();
	CurNode = aNode;
	NumOfRemElmnts = (*aSeq).NumOfElements();

	NumIsMaxSubsetOf++;

	if( (*CurNode).Level < NumOfRemElmnts )
		return false;
	
	while( true )
	{
		if( (*anItemSet).IsSubsetOf((*CurNode).Item) )
		{
			CurNode = (*CurNode).Parent;
			(*aSeq).MovePrev();
			anItemSet = (*aSeq).Current();
			NumOfRemElmnts--;
			if( NumOfRemElmnts == 0 )
				return true;

		} else {
			CurNode = (*CurNode).Parent;
			if( (*CurNode).Level < NumOfRemElmnts )
				return false;
		}
	}
}

// Returns the item in the anItemSet, with shortest header list.
int SeqTree::LeastFreqInHeader( ItemSet * anItemSet )
{
	int anItem;
	int Res = -1;
	int Min = 10000000;
	int HeaderSize;

	for( int i=0; i<(*anItemSet).NumOfItems(); i++ )
	{
		anItem = (*anItemSet).Items[i];
		HeaderSize = Header[anItem].size();
		if( HeaderSize < Min )
		{
			Min = HeaderSize;
			Res = anItem;
		}
		if( Min==0 ) break;
	}

	return Res;
}

#if defined( _USE_STRING_ELEMINATION )
bool SeqTree::IsContained( const struct PROJ_DB *proj_db, int * aSeqPtr )
{
	Sequence * aSeq;
	SeqWrap * aSeqWrap;

	aSeqWrap = new SeqWrap( aSeqPtr );

	if( !(*aSeqWrap).IsEmpty() )
	{
		aSeq = new Sequence( proj_db, aSeqPtr );
		return IsContained( aSeq );
	} else {
		return false;
	}
}
#endif // defined( _USE_STRING_ELEMINATION )

bool SeqTree::IsContained( Sequence * aSeq )
{
	int anItem;
	ItemSet * LastItemSet;
  NodeVector::iterator Itrtr;
  //NodeList::iterator Itrtr;

	if( (*aSeq).NumOfElements() == 0 ) return true;

	(*aSeq).MoveLast();
	LastItemSet = (*aSeq).Current();

	// Just check for the item in the item list with shortest header list.
	anItem = LeastFreqInHeader(LastItemSet);
	if( Header[anItem].size() == 0 ) return false; 
	for( Itrtr = Header[anItem].begin(); Itrtr != Header[anItem].end(); Itrtr++)
	{
		// Header list is sorted based on the level, 
		// so stop as soon as level is less than # elements.
		if( (*(*Itrtr)).Level < (*aSeq).NumOfElements() ) // 123
			break; // 123

		if( IsSubsetOfBranch( aSeq, (*Itrtr) ) )
			return true;
	}

	return false;
}

// If aSeq is a superset of the sequence in the tree with aNode as its last element.
bool SeqTree::IsSupersetOfBranch( Sequence * aSeq, TreeNode * aNode )
{
	int NumOfRemElmnts;
	ItemSet * anItemSet;
	TreeNode * CurNode;

	NumIsMaxSubsetOf++;

	if( (*aNode).NumOfChildren() > 0 || (*aNode).Level > (*aSeq).NumOfElements() )
		return false;
						
	(*aSeq).MoveLast();
	anItemSet = (*aSeq).Current();
	CurNode = aNode;
	NumOfRemElmnts = (*aSeq).NumOfElements();

	if( (*CurNode).Level > NumOfRemElmnts )
		return false;
	
	while( true )
	{
		if( (*CurNode).Level == 0 )
			return false;
		if( (*(*CurNode).Item).IsSubsetOf( anItemSet ) )
		{
			CurNode = (*CurNode).Parent;
			(*aSeq).MovePrev();
			anItemSet = (*aSeq).Current();
			NumOfRemElmnts--;
			if( (*CurNode).Level == 0 )
				return true;

		} else {
			(*aSeq).MovePrev();
			anItemSet = (*aSeq).Current();
			NumOfRemElmnts--;
			if( (*CurNode).Level > NumOfRemElmnts )
				return false;
		}
	}
}

void SeqTree::PrintHeaderList( int anItem )
{
	printf(" ======= Size of the Header list for item %d =%d \n", anItem, Header[anItem].size() );
  NodeVector::iterator myIt;
  NodeVector::iterator BIt;
  NodeVector::iterator EIt;
  //NodeList::iterator myIt;
  //NodeList::iterator BIt;
  //NodeList::iterator EIt;
	BIt = Header[anItem].begin();
	EIt = Header[anItem].end();
	EIt--;
	//printf(" Head = " );
	//(*(*(*BIt)).Item).Print();
	//printf("\n End = " );
	//(*(*(*EIt)).Item).Print();
	//printf("\n" );
	for( myIt = Header[anItem].begin(); myIt != Header[anItem].end(); myIt++)
	{
		try 
		{
			(*(*(*myIt)).Item).Print();
			printf( "%d %d", (*(*myIt)).Level, (*myIt) ); 
		}
		catch(...) 
		{
			printf("*Err*" );
		}

	}
	printf("\n =================== End\n" );
}

void SeqTree::DeleteSubsets( Sequence * aSeq )
{
	int anItem;
	ItemSet * anItemSet;
  ElementVector::iterator ElementItrtr;
  NodeVector::iterator Itrtr;
  //NodeList::iterator Itrtr;
  NodeVector::iterator tmpItrtr;
  //NodeList::iterator tmpItrtr;

	// For each element, anItemSet, in aSeq.
	for( ElementItrtr = (*aSeq).Elements.begin(); ElementItrtr != (*aSeq).Elements.end(); ElementItrtr++)
	{
		anItemSet = (*ElementItrtr);

		// For each item, anItem, in anItemSet.
		for( int i=0; i<(*anItemSet).NumOfItems(); i++ )
		{
			anItem = (*anItemSet).Items[i];
			if( Header[anItem].size() > 0 )
			{
				Itrtr = Header[anItem].end() - 1;
				while( Itrtr != Header[anItem].begin() - 1 )
				{
					try 
					{
						if( (*(*Itrtr)).Level > (*aSeq).NumOfElements() ) break; //123
						
						if( IsSupersetOfBranch( aSeq, (*Itrtr) ) )
						{
							tmpItrtr = Itrtr;
							tmpItrtr--;

							NumOfDels++;
							DeleteSequence( (*Itrtr) );
							Itrtr = tmpItrtr;
						} else 
							Itrtr--;
					} catch(...)	{
						Itrtr--;
						printf( " Error in Delete Subsets in SeqTree.\n" );
					}
				}

			}
		}
	}
}


int SeqTree::AddSeq( Sequence * aSeq )
{
	TreeNode * tmp;
	TreeNode * Current;
	ItemSet * anItemSet;
	bool * Duplicated;
	int anItem;
	int CurrentLevel;

	//printf("==============>");
	//(*aSeq).Print();
	NumOfAddIfClosed++;
	// aSeq was not added.
	if( IsContained( aSeq ) ) return 1;

	DeleteSubsets( aSeq );

	Current = Root;
	(*Current).Sup = (*aSeq).Sup;

	Duplicated = (bool *)calloc( NumOfItems, sizeof(bool) );

	(*aSeq).MoveFirst();

	CurrentLevel = 1;

	while( !(*aSeq).IsLast() )
	{
		anItemSet = (*aSeq).Current();

		tmp = (*Current).FindChild( anItemSet );


		if( !tmp )
		{ // Add a new node to tree
			tmp = new TreeNode( anItemSet );
			(*tmp).Parent = Current;
			(*tmp).Level = CurrentLevel;

			(*Current).AddChild( tmp );
		}

		Current = tmp;
		(*Current).Sup = (*aSeq).Sup;

		(*aSeq).MoveNext();
		CurrentLevel++;
	}
	(*Current).Sup = (*aSeq).Sup;

	// Add the last instance of each item in the sequence to the header list.
	while( Current != Root )
	{
		anItemSet = (*Current).Item;

		for( int i=0; i<(*anItemSet).NumOfItems(); i++ )
		{
			anItem = (*anItemSet).Items[i];
			if( !Duplicated[ anItem ] )
			{
				Header[anItem].push_back( Current );
				// To keep the Header lists sorted.
				inplace_merge( Header[anItem].begin(), Header[anItem].end()-1, Header[anItem].end(), TreeNodeLevelLess ); //123
			}

			Duplicated[ anItem ] = true;
		}
		Current = (*Current).Parent;
	}

	// aSeq was added.
	NumOfAdds++;
	return 0;
}

void SeqTree::Print( FILE * aFile )
{
  NodeVector::iterator Itrtr;
  //NodeList::iterator Itrtr;

	if( Root )
		(*Root).Print( aFile );

	fprintf( aFile, "\n Header Table:\n" );

	for( int i=0; i<NumOfItems; i++ )
	{

		fprintf( aFile, "  List for %d (Len=%d) => ", i, Header[i].size() );
		for( Itrtr = Header[i].begin(); Itrtr != Header[i].end(); Itrtr++)
		{
			fprintf( aFile, "  " );
			if( (*(*Itrtr)).Item ) (*(*(*Itrtr)).Item).Print();
		}
		fprintf( aFile, " \n" );
	}
}

void SeqTree::InternalPrintRules( FILE * aFile, TreeNode * InNode, Sequence * aSeq, SeqList * SortedSeqList )
{

	TreeNode * aNode;
	TreeNode * aChild;

	if( InNode )
		aNode = InNode;
	else {
		aNode = Root;
		NumOfSeqs = 0;
	}

	if( !aSeq )
		aSeq = new Sequence();


	if( !aNode ) return;

	if( (*aNode).NumOfChildren() == 0 )
	{
		(*aSeq).Sup = (*aNode).Sup;

		
		if( SortedSeqList == NULL )
		{
			(*aSeq).Print( aFile );
		} else {
			(*SortedSeqList).push_back( new Sequence( aSeq ) );
			// To keep the List vector sorted.
			inplace_merge( (*SortedSeqList).begin(), (*SortedSeqList).end()-1, (*SortedSeqList).end(), SeqGreater );
			//(*(*(*SortedSeqList).begin())).Print();
		}
		
		NumOfSeqs++;
	}

	//fprintf( aFile, "\n Rules:\n" );

	(*aNode).MoveFirst();
	while( !(*aNode).IsLast() )
	{
		aChild = (*aNode).Current();
		//(*(*aChild).Item).Print( aFile );
		if( aChild )
		{
			(*aSeq).Add( (*aChild).Item );
			InternalPrintRules( aFile, aChild, aSeq, SortedSeqList );
		}
		(*aSeq).Del();
		(*aNode).MoveNext(); 
		//if( !(*aNode).IsLast() )
			//fprintf( aFile, " Support = %d\n", (*aNode).Sup );
	}
	if( (*aSeq).NumOfElements() == 0 )
	{
		//fprintf( aFile, " Support = %d\n", (*aNode).Sup );
		delete aSeq;
	}
}

void SeqTree::PrintRules( FILE * aFile, TreeNode * InNode, Sequence * aSeq )
{
	
	SeqList * SortedSeqList = NULL;

	#if defined( _SORT_RESULTS )
		SeqList::iterator ListItrtr;
		SortedSeqList = new SeqList;
	#endif

	InternalPrintRules( aFile, InNode, aSeq, SortedSeqList );

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
	fprintf( aFile, "# of times AddIfMaxSeq called %d\n", NumOfAddIfClosed );
	fprintf( aFile, "# of times IsMaxSubsetOf called %d\n", NumIsMaxSubsetOf );
	fprintf( aFile, "# of elements in the list %d\n", NumOfSeqs );

}

SeqTree::~SeqTree()
{
	delete[] Header;
	delete Root;
}
//************************
//************************

//************************
//************************
TreeNode::TreeNode( ItemSet * anItemSet, int Support )
{
	Item = anItemSet;
	Sup = Support;
	Parent = NULL;
	Level = 0;
}

bool TreeNodeLess( TreeNode * a, TreeNode * b )
{
	return (*(*a).Item).IsLessThan( (*b).Item ) ;
}

void TreeNode::AddChild( TreeNode * Child ) 
{ 
	Children.push_back(Child); 
	// To keep the children vector sorted.
	inplace_merge( Children.begin(), Children.end()-1, Children.end(), TreeNodeLess );
}

int TreeNodeCompare( TreeNode ** a, TreeNode ** b )
{
	return (*(*(*a)).Item).Compare( (*(*b)).Item );
}

TreeNode * TreeNode::FindChild( ItemSet * anItem )
{
	TreeNode ** Res;
	TreeNode * tmp = new TreeNode(anItem);

	Res = (TreeNode **) bsearch( &tmp, Children.begin(), Children.size(), sizeof( TreeNode *), (int (*)(const void*, const void*))TreeNodeCompare );

	if( Res )
		return (*Res);

	return NULL;
}

void TreeNode::DelChild( TreeNode * Child )
{
	TreeNode ** Res;

	if( NumOfChildren() == 1 )
	{
		Res = Children.begin();
	} else {
		Res = (TreeNode **) bsearch( &Child, Children.begin(), Children.size(), sizeof( TreeNode *), (int (*)(const void*, const void*))TreeNodeCompare );
	}

	if( Res )
	{
		Children.erase( (Res) );
		//return (*Res);
	}

}

void TreeNode::Print( FILE * aFile )
{
  NodeVector::iterator Itrtr;

	fprintf( aFile, "  Node = " );
	if( Item ) (*Item).Print();

	fprintf( aFile, "  Level=%d ", Level );

	fprintf( aFile, " Suport = %d ) ", Sup );

	fprintf( aFile, "   Children " );

	for( Itrtr = Children.begin(); Itrtr != Children.end(); Itrtr++)
	{

		if (Itrtr != Children.end()-1 )
		{
			if( (*(*Itrtr)).Item ) (*((*(*Itrtr)).Item)).Print();
			fprintf( aFile, " ," );
		} else {
			if( (*(*Itrtr)).Item ) (*((*(*Itrtr)).Item)).Print();
		}
	}

	fprintf( aFile, "\n" );
	for( Itrtr = Children.begin(); Itrtr != Children.end(); Itrtr++)
	{
		//fprintf( aFile, "    " );
		(*(*Itrtr)).Print( aFile );
	}

	//fprintf( aFile, "\n" );
}


TreeNode::~TreeNode()
{
  NodeVector::iterator Itrtr;

	if( Item ) 
	{
		delete Item;
		Item = NULL;
	}

	for( Itrtr = Children.begin(); Itrtr != Children.end(); Itrtr++)
		delete *Itrtr;

}
//************************
//************************

//************************
//************************
#else //if defined( _USE_MAX_TREE )
//************************
//************************

SeqTree::SeqTree( int dummy )
{
	NumOfAdds = 0;
	NumOfDels = 0;
	NumOfAddIfMax = 0;
	NumIsMaxSubsetOf = 0;
}

inline void SeqTree::Add( Sequence * aSeq )
{
	List.insert( List.end(), aSeq );
	NumOfAdds++;
}

inline SeqList::iterator SeqTree::Del( SeqList::iterator anItr )
{
	NumOfDels++;
	return List.erase( anItr );
}

// Tries to add aSeq to the list if non of the list elements
// is a closed superset of aSeq.
// Sequences in the list that are Max subset of aSeq
// will be deleted from the list.
// Returns true if aSeq is added.
bool SeqTree::AddSeq( Sequence * aSeq )
{
	SeqList::iterator Itrtr;
	bool IsSuper = false;

	NumOfAddIfMax++;
	for( Itrtr = List.begin(); Itrtr != List.end(); )
	{
		if( !IsSuper )
		{
			NumIsMaxSubsetOf++;
			if( (*aSeq).IsMaxSubsetOf( (*Itrtr) ) )
				return false;
		}

		NumIsMaxSubsetOf++;
		if( (*(*Itrtr)).IsMaxSubsetOf( aSeq ) )
		{

			delete (*Itrtr) ;

			IsSuper = true;
			Itrtr = Del( Itrtr );
		} else
			Itrtr++;
	}

	Add( aSeq );
	return true;
}

bool SeqTree::IsContained( const struct PROJ_DB *proj_db, int * aSeqPtr )
{
	bool temp;
	Sequence * aSeq;

	aSeq = new Sequence( proj_db, aSeqPtr );

	temp = IsContained( aSeq );
	//(*aSeq).Print();
	//if( temp )
	//	printf( "     Is contained in the resultset.\n" );
	//else
	//	printf( "     Is NOT contained in the resultset.\n" );

	return temp;
}

bool SeqTree::IsContained( Sequence * aSeq )
{
	SeqList::iterator Itrtr;


	for( Itrtr = List.begin(); Itrtr != List.end(); )
	{
		NumIsMaxSubsetOf++;
		if( (*aSeq).IsMaxSubsetOf( (*Itrtr) ) )
			return true;
	}

	return false;
}

void SeqTree::PrintRules( FILE *aFile )
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
	fprintf( aFile, "# of times AddIfMaxSeq called %d\n", NumOfAddIfMax );
	fprintf( aFile, "# of times IsMaxSubsetOf called %d\n", NumIsMaxSubsetOf );
	fprintf( aFile, "# of elements in the list %d\n", List.size() );

}

SeqTree::~SeqTree()
{
	SeqList::iterator Itrtr;

	for( Itrtr = List.begin(); Itrtr != List.end(); Itrtr++)
		delete *Itrtr;
}
//************************
//************************
#endif // defined( _USE_MAX_TREE )


//************************
//************************

#endif // defined( _FIND_MAX_SEQS )




