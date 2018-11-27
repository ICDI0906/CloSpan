#if !defined( _CLOSED_SEQTREE_H__ )
#define _CLOSED_SEQTREE_H__

#include <stdio.h> 
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

class Sequence;
class SequenceList;


class Sequence
{
	private:
		inline int * IsItemSetSubsetOf( int * ThisStart, int * SeqStart, Sequence * aSeq );
	public:
#if defined( _USE_OUTPUT_BUFFER )
		Sequence * Parent;
		int ReferenceCount;
		void EmptyBuffer( SequenceList * aList );
#endif
		int Support;
		int * StartPtr;
		int Len;

		Sequence();
		Sequence( int * Ptr, int Len, int Sup );

		inline bool EndOfSequence( int * aPtr );
		inline bool EndOfItemSet( int * aPtr );
		inline int NumOfItemSets();
		bool IsClosedSubsetOf( Sequence * aSeq ); // This method is used to find closed sequences.

		void Print( FILE * aFile = stdout );
		~Sequence();
};

#if !defined( _NEW_SEQUENCE_LIST )

typedef vector<Sequence *> SeqList;

class SequenceList
{
	private:
		int NumOfAdds;
		int NumOfDels;
		int NumOfAddIfClosed;
		long NumIsClosedSubsetOf;
	public:
		SeqList List;

		SequenceList();

		inline void AddSeq( Sequence * aSeq );
		bool AddIfClosedSeq( Sequence * aSeq );
		inline SeqList::iterator Del( SeqList::iterator anItr );
		void Print( FILE * aFile = stdout );

		~SequenceList();
};

#else // !defined( _NEW_SEQUENCE_LIST )
	typedef vector<Sequence *> SeqList;

#if _NEW_SEQUENCE_LIST < 20

#if _NEW_SEQUENCE_LIST > 0
	typedef map< int, SeqList *, greater<int> > SeqMap;
#endif

class SequenceList
{
	private:
		int NumOfAdds;
		int NumOfDels;
		int NumOfAddIfClosed;
		int NumIsClosedSubsetOf;
	public:
		#if _NEW_SEQUENCE_LIST > 0
			SeqMap List;
		#else
			SeqList List;
		#endif

		SequenceList();

		inline void AddSeq( Sequence * aSeq );
		bool AddIfClosedSeq( Sequence * aSeq );
		inline SeqList::iterator Del( SeqList::iterator anItr );
		void Print( FILE * aFile = stdout );

		~SequenceList();
};
#else // _NEW_SEQUENCE_LIST < 20
//***************************
//***************************

class TreeNode;
class SequenceList;
typedef vector<TreeNode *> NodeVector;

class TreeNode
{
	private:
	public:
		NodeVector * Children;
		TreeNode * Parent;
		int Item;
		bool ItemIsIntra;
		int ItemsetNumber;
		int Support;

		TreeNode( int anItem = -1, bool ItemIsIntra = false, int Sup = 0, TreeNode * aParent = NULL );

		inline void DelChild( TreeNode * Child );
		inline TreeNode * FindChild( int anItem, bool Intra );
		inline TreeNode * FindChild( TreeNode * Child );

		TreeNode * AddChild( TreeNode * Child );
		inline int NumOfChildren();
		inline int MaxChildSupport();
		bool LastItemOfSequence();
		void PrintRules( FILE * aFile, int * SeqBuf, int SeqLen, SequenceList * aSeqTree, SeqList * aSeqList, int NumOfItems );
		void Print( char * PrefixString = "", FILE * aFile = stdout );

		~TreeNode();
};

class SequenceList
{
	friend class TreeNode;
	private:
		TreeNode * Root;
		int NumOfItems;

		int NumOfAdds;
		int NumOfDels;
		int NumOfAddIfClosed;
		int NumIsClosedSubsetOfBranch;
		int NumIsClosedSupersetOfBranch;
		int Size; // Is used by PrintRules method.
		long NumOfTreeNodes;
		long PeakNumOfTreeNodes;

		inline void IncNumOfTreeNodes();
		inline void DecNumOfTreeNodes();
		void InternalPrintRules( FILE * aFile );
		inline void UpdateHeaderList( TreeNode * LastNode );
		void RemoveFromHeaderList( TreeNode * aNode );
		inline NodeVector::iterator FindInHeaderList( TreeNode * aNode );


	public:
		NodeVector * Header;

		SequenceList( int ItemsCount );
		
		bool IsClosedSubsetOfBranch( Sequence * aSeq, TreeNode * aNode );
		bool IsClosedSupersetOfBranch( Sequence * aSeq, TreeNode * aNode );
		int LeastFreqInHeader( Sequence * aSeq );
		bool IsContained( Sequence * aSeq );
		void DeleteSequence( TreeNode * aNode );
		void DeleteClosedSubsets( Sequence * aSeq );
		int AddIfClosedSeq( Sequence * aSeq );
		void Print( FILE * aFile = stdout );
		void PrintTree( FILE * aFile = stdout );
		void PrintHeaderList( int anItem );

		~SequenceList();
};


//***************************
//***************************

#endif // _NEW_SEQUENCE_LIST < 20

#endif // !defined( _NEW_SEQUENCE_LIST )



void RTATest( const struct PROJ_DB * pDB );


#endif // !defined( _CLOSED_SEQTREE_H__ )