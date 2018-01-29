
#ifndef PAGE_H
#define PAGE_H

#include <string>
using namespace std;

class MyDB_Page {

public:
    
    MyDB_Page();
    
    //PageID, pageSize, pinned, type
    MyDB_Page(string, size_t, string, bool, int);
    
    MyDB_Page(string);

    ~MyDB_Page();

    void writeData(char *);
    
    char * getBytes();
    
    int getType();
    
    string getPageId();
    
    void addHandleCnt();
    
    int subHandleCnt();
    
    bool isThisDBPage(string, long);
    
    void setDBInfo(string, long);
    
    void setWritten(bool);

    bool isWritten();

    string toString();
    
    bool pin();

    bool unpin();
private:

    string pageId;
    
    size_t pageSize;
    
    // 1 for table, 0 for anonymous
    int type;
    
    bool pinned;
    
    int handleCnt;
    
    char* pageData;
    
    string dbName;
    
    long dbPageId; 

    bool writeFlag;

};

#endif
