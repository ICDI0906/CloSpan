#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>


using namespace std;


class ItemSet;
class TreeNode;
class SeqTree;
typedef vector<TreeNode *> NodeVector;
typedef vector<ItemSet *> ElementVector;
typedef list<TreeNode *> NodeList;

class ItemSet
{
	private:
		int Count;
	public:
		int * Items;

		ItemSet( int NumOfItems, int * Items = NULL );
		ItemSet( ItemSet * anItemSet );

		inline bool IsSubsetOf( ItemSet * anItemSet );
		inline bool IsLessThan( ItemSet * anItemSet );
		inline int Compare( ItemSet * anItemSet );

		void Add ( int Item );
		void Print( FILE * aFile = stdout );
		void Print1( FILE * aFile = stdout );
		int NumOfItems() {  return Count; };

		~ItemSet();
};
 

class Sequence
{
	private:
    ElementVector::iterator Iterator;
	public:
		ElementVector Elements;
		int Sup;

		Sequence();
		Sequence( int * Ptr, int Sup = 0 );
		Sequence( int * Pat, int PatLen, int Sup = 0 );
		Sequence( Sequence * aSeq );
		Sequence( const struct PROJ_DB *proj_db, int Sup = 0 );
		Sequence( const struct PROJ_DB *proj_db, int * aSeqPtr );

		#if !defined( _USE_MAX_TREE )
			bool IsMaxSubsetOf( Sequence * aSeq );
		#endif // !defined( _USE_MAX_TREE )
		inline ItemSet * Current() { return *Iterator; };
		inline void MoveNext() { if( Iterator != Elements.end() ) Iterator++; };
		inline void MovePrev() { if( Iterator != Elements.begin() ) Iterator--; };
		inline void MoveFirst() { Iterator = Elements.begin(); };
		inline void MoveLast() { Iterator = Elements.end() - 1; };
		inline bool IsFirst() { return Iterator == Elements.begin(); };
		inline bool IsLast() { return Iterator == Elements.end(); };
		//ElementVector::iterator StartAddress() { return Elements.begin(); };
		inline int NumOfElements() { return Elements.size(); };
		inline void Add( ItemSet * Element ) { Elements.push_back(Element); };
		void Add( struct PROJ_SEQ *m_pProjSeq );
		void Del( ) { Elements.pop_back(); };
		void Print( FILE * aFile = stdout );
		~Sequence();
};

typedef vector<Sequence *> SeqList;

#if defined( _USE_MAX_TREE )
class TreeNode
{
	private:
		NodeVector Children;
    NodeVector::iterator Iterator;
	public:
		TreeNode * Parent;
		int Sup; 
		ItemSet * Item;

		int Level;  // Node level in the tree;

		TreeNode( ItemSet * anItem = NULL, int Support = 0 );

		TreeNode * FindChild( ItemSet * anItem );

		TreeNode * Current() { return *Iterator; };
		void MoveNext() { if( Iterator != Children.end() ) Iterator++; };
		void MoveFirst() { Iterator = Children.begin(); };
		void MoveLast() { Iterator = Children.end() - 1; };
		bool IsFirst() { return Iterator == Children.begin(); };
		bool IsLast() { return Iterator == Children.end(); };
		int NumOfChildren() { return Children.size(); };
		void AddChild( TreeNode * Child );
		void DelChild( TreeNode * Child );
		void Print( FILE * aFile = stdout );

		~TreeNode();
};

class SeqTree
{
	private:
		TreeNode * Root;
		int NumOfItems;
		int NumOfSeqs;
		int NumOfAdds;
		int NumOfDels;
		int NumOfAddIfClosed;
		long NumIsMaxSubsetOf;
		void InternalPrintRules( FILE * aFile = stdout, TreeNode * aNode = NULL, Sequence * aSeq = NULL, SeqList * SortedSeqList = NULL );
	public:
		NodeVector * Header;
		//NodeList * Header;

		SeqTree( int ItemsCount );
		
		int LeastFreqInHeader( ItemSet * anItemSet ); // Returns the item in the anItemSet, with shortest header list.
		bool IsSubsetOfBranch( Sequence * aSeq, TreeNode * aNode ); // If aSeq is a subset of the sequence in the tree with aNode as its last element.
		bool IsSupersetOfBranch( Sequence * aSeq, TreeNode * aNode ); // If aSeq is a superset of the sequence in the tree with aNode as its last element.
		void RemoveFromHeaderList( TreeNode * aNode );
		void DeleteSequence( TreeNode * aNode ); // Deletes the sequence ended by aNode from the tree.
		int AddSeq( Sequence * aSeq );
		void DeleteSubsets( Sequence * aSeq );

		#if defined( _USE_STRING_ELEMINATION )
			bool IsContained( const struct PROJ_DB *proj_db, int * aSeq );
		#endif // defined( _USE_STRING_ELEMINATION )

		bool IsContained( Sequence * aSeq );
		void Print( FILE * aFile = stdout );
		void PrintRules( FILE * aFile = stdout, TreeNode * aNode = NULL, Sequence * aSeq = NULL );
		void PrintHeaderList( int anItem );

		~SeqTree();
};


#else // if defined( _USE_MAX_TREE )


class SeqTree
{
	private:
		int NumOfAdds;
		int NumOfDels;
		int NumOfAddIfMax;
		long NumIsMaxSubsetOf;

		inline SeqList::iterator Del( SeqList::iterator anItr );
		inline void Add( Sequence * aSeq );
	public:
		SeqList List;

		SeqTree( int dummy = 0 );

		bool AddSeq( Sequence * aSeq );
		bool IsContained( const struct PROJ_DB *proj_db, int * aSeq );
		bool IsContained( Sequence * aSeq );
		void PrintRules( FILE * aFile = stdout );

		~SeqTree();
};

#endif // defined( _USE_MAX_TREE )
