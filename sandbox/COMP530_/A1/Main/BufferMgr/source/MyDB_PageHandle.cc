
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include <iostream>
#include "MyDB_PageHandle.h"
#include "MyDB_BufferManager.h"

char * MyDB_PageHandleBase :: getBytes () {
	return pagePtr->getBytes();
}

void MyDB_PageHandleBase :: wroteBytes () {
    isWritten = true;
    pagePtr->setWritten(true);
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
    int action = pagePtr->subHandleCnt();
    //std::cout << "unpinPage action: " << action << std::endl;
    if(action == 1){
        bufferMgr->unpinPage(pagePtr);
    }else if(action == -1){
        //cout << "release page action " << pagePtr->getPageId() <<endl;
        bufferMgr->releasePage(pagePtr);
        pagePtr = nullptr;
    }
}

MyDB_PageHandleBase :: MyDB_PageHandleBase(MyDB_BufferManager * bm, MyDB_Page *pagePointer) {
    bufferMgr = bm;
    //pageId = i;
    isWritten = false;
    pagePtr = pagePointer;
    pagePtr->addHandleCnt();

}

MyDB_Page* MyDB_PageHandleBase :: getPagePtr(){
    return pagePtr;
}
#endif

