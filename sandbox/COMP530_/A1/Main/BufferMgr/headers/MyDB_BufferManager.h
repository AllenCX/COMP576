
#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"
#include "MyDB_Page.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <utility>
#include <set>

using namespace std;

struct LRUNode{
    LRUNode *next;
    LRUNode *prev;
    pair<MyDB_TablePtr, string> key;
    MyDB_Page *pageLoc; //actually is the page's location on buffer
};
class MyDB_BufferManager {

public:

	// THESE METHODS MUST APPEAR AND THE PROTOTYPES CANNOT CHANGE!

	// gets the i^th page in the table whichTable... note that if the page
	// is currently being used (that is, the page is current buffered) a handle 
	// to that already-buffered page should be returned
	MyDB_PageHandle getPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page that will no longer exist (1) after the buffer manager
	// has been destroyed, or (2) there are no more references to it anywhere in the
	// program.  Typically such a temporary page will be used as buffer memory.
	// since it is just a temp page, it is not associated with any particular 
	// table
	MyDB_PageHandle getPage ();

	// gets the i^th page in the table whichTable... the only difference 
	// between this method and getPage (whicTable, i) is that the page will be 
	// pinned in RAM; it cannot be written out to the file
	MyDB_PageHandle getPinnedPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page, like getPage (), except that this one is pinned
	MyDB_PageHandle getPinnedPage ();

	// un-pins the specified page
	void unpin (MyDB_PageHandle unpinMe);

	// creates an LRU buffer manager... params are as follows:
	// 1) the size of each page is pageSize 
	// 2) the number of pages managed by the buffer manager is numPages;
	// 3) temporary pages are written to the file tempFile
	MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile);
	
	// when the buffer manager is destroyed, all of the dirty pages need to be
	// written back to disk, any necessary data needs to be written to the catalog,
	// and any temporary files need to be deleted
	~MyDB_BufferManager ();

	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS
    
    //search DB page, return page ID
    int searchDbPageId(MyDB_TablePtr, long);
    
    int searchIdInBuffer(int);   
    
    //read Id from Disk, filename, id
    MyDB_Page* loadPagefromDisk(string, string);
    
    void writeToDisk(MyDB_Page *, string);

    void _removeFromLRU(LRUNode *node, bool);
    
    void _addToLRU(LRUNode *node);

    void _LRUPut(pair<MyDB_TablePtr, long>, MyDB_Page*);

    void releasePage(MyDB_Page* releaseMe);
    void unpinPage(MyDB_Page*);
    void scanTempFile(MyDB_TablePtr);
private:

	// YOUR STUFF HERE
    MyDB_Page * buffer;
    int file;
    size_t pageSize;
    unsigned int numPages;
    string tempFileName;
    int maxPageId;
    
    int occupied;
    
    //for buffer slot quick look up
    // index is page ID, value is slot Num
    //vector <int> pageLoc;
    
    // index is slot ID, value is pageID
    vector <int> bufferUsage;
    
    //for LRU
    list <LRUNode> LRUList;
    
    
    //To record the location of table page
    //LRUMap records those unpinned pages
    map <pair<MyDB_TablePtr, string>, LRUNode *> LRUMap;
    // store all key page values
    map <pair<MyDB_TablePtr, string>, MyDB_Page *> tablePageMap;
    // in RAM (true) or in Disk (false)
    map <pair<MyDB_TablePtr, string>, bool> pageStatus;

    // those pages are in the RAM 
    map <pair<MyDB_TablePtr, string>, MyDB_Page*> pinnedPageMap;
    set <MyDB_Page*> emptySlot;
    map <MyDB_Page*, MyDB_TablePtr> pageTableMap;

    map <string, MyDB_Page*> anonymousPageMap;
    LRUNode *dummyHead, *dummyTail;

    set<string> scanSet;
};

#endif


