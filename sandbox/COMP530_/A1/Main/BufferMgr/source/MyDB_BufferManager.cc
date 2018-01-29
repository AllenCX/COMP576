
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include "MyDB_Page.h"
#include <string>
//#include <malloc.h>
#include <malloc/malloc.h>
#include <stdlib.h>   
#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr tablePtr, long i) {
    string pid = string(tablePtr->getName()) + "-" + to_string(i);
    cout << "pid: " << pid << endl;
    pair<MyDB_TablePtr, string> key = pair<MyDB_TablePtr, string>(tablePtr, pid);

    
    if(scanSet.count(tablePtr->getName()) == 0) {
        cout << "scanning file at: " << tablePtr->getStorageLoc() << endl;
        scanSet.insert(tablePtr->getName());
        scanTempFile(tablePtr);
    }
    
    MyDB_Page *pageLoc, *page;

    if(pinnedPageMap.count(key) > 0 && pageStatus[key] == true){
        cout << "pinned and in the RAM" << endl;
        MyDB_Page *pageLoc = pinnedPageMap[key];
        return make_shared <MyDB_PageHandleBase>(this, pageLoc); 
    }

    
    if(LRUMap.count(key) > 0){
        // in RAM
        cout << "In Ram unpinned" << endl;
        page = LRUMap[key]->pageLoc;
        emptySlot.insert(page);
        //LRUMap.erase[key];
        _removeFromLRU(LRUMap[key], true);
    }

    if(pageStatus.count(key) > 0 && pageStatus[key] == false){
        cout << "In temp file" << endl;
        page = loadPagefromDisk(tablePtr->getStorageLoc(), pid);
    }
    if(pageStatus.count(key) == 0){
        cout << "new one " << endl;
        page = new MyDB_Page(pid, pageSize, tablePtr->getName(), false, 1);
    }

    
    if(emptySlot.size() == 0){
        cout << "No more space evicting page\n";
        LRUNode *node = dummyHead->next;
        //memcpy(tempPage, node->pageLoc, sizeof(MyDB_Page));
        //MyDB_Page *tempPage = (MyDB_Page *)malloc(sizeof(MyDB_Page));
        cout << "evicting page: " << node->pageLoc->toString();
        emptySlot.insert(node->pageLoc);

        if(node->pageLoc->isWritten()){
            cout << "write to disk key:" << key.second <<endl;
            writeToDisk(node->pageLoc, pageTableMap[node->pageLoc]->getStorageLoc());
            pageStatus[key] = false;
        }

        LRUMap.erase(node->key);
        _removeFromLRU(node, true);
        //exit(0);
    }

    pageLoc = (MyDB_Page *)memcpy(*emptySlot.begin(), page, sizeof(MyDB_Page));
    emptySlot.erase(pageLoc);

    pageStatus[key] = true;
    pageTableMap[pageLoc] = tablePtr;

    LRUNode *node = new LRUNode;
    node->pageLoc = pageLoc;
    node->key = key;
    LRUMap[key] = node;

    _addToLRU(node);

    
    cout << "size of slots: " << emptySlot.size() << endl;
    cout << "size of pinnedPageMap: " << pinnedPageMap.size() << endl;
    cout << "size of LRUMap: " << LRUMap.size() << endl;
    
   
    return make_shared <MyDB_PageHandleBase>(this, pageLoc);
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {

    string pid = to_string(-rand() % 20000 - 1);
    while(anonymousPageMap.count(pid) > 0){
        pid = to_string(-rand() % 20000 - 1);
    }
    cout << "anonymous pid: " << pid << endl;
    pair<MyDB_TablePtr, string> key = pair<MyDB_TablePtr, string>(nullptr, pid);

    MyDB_Page *page = new MyDB_Page(pid, pageSize, "anonymous", false, 0);
    if(pinnedPageMap.size() + LRUMap.size() == numPages){ 
        LRUNode *node = dummyHead->next;
        cout << "evicting page: " << node->pageLoc->toString() << endl;
        emptySlot.insert(node->pageLoc);

        if(node->pageLoc->isWritten()){
            cout << "write to disk key:" << key.second <<endl;
            writeToDisk(node->pageLoc, pageTableMap[node->pageLoc]->getStorageLoc());
            pageStatus[key] = false;
        }

        LRUMap.erase(node->key);
        _removeFromLRU(node, false);
    }

    MyDB_Page *pageLoc = (MyDB_Page *)memcpy(*emptySlot.begin(), page, sizeof(MyDB_Page));
    //cout << "anonymousPage " << pageLoc->toString();
    emptySlot.erase(*emptySlot.begin());
    delete page;
    pageStatus[key] = true;

    /*
    cout << "size of slots: " << emptySlot.size() << endl;
    cout << "size of pinnedPageMap: " << pinnedPageMap.size() << endl;
    cout << "size of LRUMap: " << LRUMap.size() << endl;
    */
	return make_shared <MyDB_PageHandleBase>(this, pageLoc);
}


void MyDB_BufferManager :: _removeFromLRU(LRUNode *node, bool isDelete){

    node->prev->next = node->next;
    node->next->prev = node->prev;
    if(isDelete) delete node;
}

void MyDB_BufferManager :: _addToLRU(LRUNode *node){
    LRUNode *p = dummyTail->prev;
    p->next = node;
    node->prev = p;
    dummyTail->prev = node;
    node->next = dummyTail;
}

void MyDB_BufferManager :: _LRUPut(pair<MyDB_TablePtr, long> key, MyDB_Page *node){

}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr tablePtr, long i) {
    // We only have two situations: 1. In Ram. 2. Not in RAM, we new one.
    
    string pid = string(tablePtr->getName()) + "-" + to_string(i);
    cout << "pid: " << pid << endl;
    pair<MyDB_TablePtr, string> key = pair<MyDB_TablePtr, string>(tablePtr, pid);
    
    if(scanSet.count(tablePtr->getName()) == 0) {
        cout << "scanning file at: " << tablePtr->getStorageLoc() << endl;
        scanSet.insert(tablePtr->getName());
        scanTempFile(tablePtr);
    }
    
    if(LRUMap.count(key) == true && pinnedPageMap.count(key) == 0){
        // in LRU but not pinned
        cout << "repin " << endl;
        LRUNode *node = LRUMap[key];
        node->pageLoc->pin();
        pinnedPageMap[key] = node->pageLoc;
        LRUMap.erase(node->key);
        _removeFromLRU(node, true);
        return make_shared <MyDB_PageHandleBase>(this, node->pageLoc);
    }

    if(pinnedPageMap.count(key) > 0 && pageStatus[key] == true){
        // if the page is in RAM just return it
        cout << "in the RAM" << endl;
        MyDB_Page *pageLoc = pinnedPageMap[key];
        pageLoc->pin();
        return make_shared <MyDB_PageHandleBase>(this, pageLoc);
    }

    MyDB_Page *page;
    if(pageStatus.count(key) > 0 && pageStatus[key] == false){
        // has been created before but not in RAM
        cout << "Load from disk" << endl;
        page = loadPagefromDisk(tablePtr->getStorageLoc(), pid);
    }else{
        cout << "new a page " << endl;
        page = new MyDB_Page(pid, pageSize, tablePtr->getName(), true, 1);
    }

    if(pinnedPageMap.size() + LRUMap.size() == numPages){ 
        //the buffer is full
        LRUNode *node = dummyHead->next;
        cout << "evicting page: " << node->pageLoc->toString() << endl;
       
        emptySlot.insert(node->pageLoc);

        if(node->pageLoc->isWritten()){
            cout << "write to disk key:" << key.second <<endl;
            writeToDisk(node->pageLoc, pageTableMap[node->pageLoc]->getStorageLoc());
            pageStatus[key] = false;
        }
        LRUMap.erase(node->key);
        _removeFromLRU(node, true);
    }
    cout << "insert at: " <<  *emptySlot.begin() << endl;
    MyDB_Page *pageLoc = (MyDB_Page *)memcpy(*emptySlot.begin(), page, sizeof(MyDB_Page));
    emptySlot.erase(*emptySlot.begin());
    delete page;
    pageStatus[key] = true;
    pinnedPageMap[key] = pageLoc;
    pageTableMap[pageLoc] = tablePtr;

    /*
    cout << "size of slots: " << emptySlot.size() << endl;
    cout << "size of pinnedPageMap: " << pinnedPageMap.size() << endl;
    cout << "size of LRUMap: " << LRUMap.size() << endl;
    */
    return make_shared <MyDB_PageHandleBase>(this, pageLoc);
    
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {

    string pid = to_string(-rand() % 20000 - 1);
    while(anonymousPageMap.count(pid) > 0){
        pid = to_string(-rand() % 20000 - 1);
    }
    cout << "anonymous pid: " << pid << endl;
    pair<MyDB_TablePtr, string> key = pair<MyDB_TablePtr, string>(nullptr, pid);

    MyDB_Page *page = new MyDB_Page(pid, pageSize, "anonymous", true, 0);
    if(pinnedPageMap.size() + LRUMap.size() == numPages){ 
        LRUNode *node = dummyHead->next;
        cout << "evicting page: " << node->pageLoc->toString() << endl;
        emptySlot.insert(node->pageLoc);

        if(node->pageLoc->isWritten()){
            cout << "write to disk key:" << key.second <<endl;
            writeToDisk(node->pageLoc, pageTableMap[node->pageLoc]->getStorageLoc());
            pageStatus[key] = false;
        }

        LRUMap.erase(node->key);
        _removeFromLRU(node, false);
    }

    MyDB_Page *pageLoc = (MyDB_Page *)memcpy(*emptySlot.begin(), page, sizeof(MyDB_Page));
    //cout << "anonymousPage " << pageLoc->toString();
    emptySlot.erase(*emptySlot.begin());
    delete page;
    pageStatus[key] = true;
    pinnedPageMap[key] = pageLoc;

    /*
    cout << "size of slots: " << emptySlot.size() << endl;
    cout << "size of pinnedPageMap: " << pinnedPageMap.size() << endl;
    cout << "size of LRUMap: " << LRUMap.size() << endl;
    */

	return make_shared <MyDB_PageHandleBase>(this, pageLoc);
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
    unpinMe->getPagePtr()->unpin();
}


void MyDB_BufferManager :: unpinPage (MyDB_Page *page) {
    //cout << "action 1 unpin pinned page\n";
    MyDB_TablePtr tablePtr = pageTableMap[page];
    page->unpin();
    cout <<"unpin in bm " << page->getPageId() <<endl;
    pair<MyDB_TablePtr, string> key = pair<MyDB_TablePtr, string>(tablePtr, page->getPageId());
    LRUNode *node = new LRUNode;
    node->key = key;
    node->pageLoc = page;
    LRUMap[key] = node;
    _addToLRU(node);
    pinnedPageMap.erase(key);
}


void MyDB_BufferManager :: releasePage(MyDB_Page* releaseMe){
    cout << "releasing\n";
    pair<MyDB_TablePtr, string> key = pair<MyDB_TablePtr, string>(nullptr, releaseMe->getPageId());
    cout << "releasing page: " << releaseMe->getPageId() << endl;
    if(LRUMap.count(key) == 0 && pinnedPageMap.count(key) == 0){
        cout << "page not in RAM" << endl;
        return;
    }
    MyDB_Page *page;
    if(pinnedPageMap.count(key) > 0){
        cout << "page in pinnedPageMap\n" << endl;
        page = pinnedPageMap[key];
        pinnedPageMap.erase(key);
        emptySlot.insert(page);
    }
    if(LRUMap.count(key) > 0){
        LRUNode *node = LRUMap[key];
        emptySlot.insert(node->pageLoc);
        _removeFromLRU(node, true);
        LRUMap.erase(key);
        pageStatus.erase(key);
    }
}

MyDB_BufferManager :: MyDB_BufferManager (size_t ps, size_t pn, string tf) {
    pageSize = ps;
    numPages = pn;
    tempFileName = tf;
    
    maxPageId = 0;
    occupied = 0;
    
    cout << "pageSize: " << pageSize << endl;
    cout << "numPages: " << numPages << endl;
    // Create a dummy page object for sizeof calculation
    cout<<"Creating Page"<<endl;
    dummyHead = new LRUNode;
    dummyTail = new LRUNode;
    dummyHead->next = dummyTail;
    dummyTail->prev = dummyHead;

    cout<<"Page size: "<<sizeof(MyDB_Page)<<endl;
    
    //Create buffer memory
    buffer = (MyDB_Page*) malloc(numPages*sizeof(MyDB_Page));

    cout << "Initializing empty slot set" << endl;
    int i = 0;
    while(i < numPages){
        emptySlot.insert(buffer + i);
        //cout << buffer + i << endl;
        i++;
    }

    cout<<"Buffer head address: "<<buffer<<endl;
    cout<<"Buffer size: "<<malloc_size(buffer)<<endl;
    
    cout << "fileName: " <<  tempFileName << endl;
    file = open(tempFileName.c_str(), O_RDONLY | O_CREAT | O_FSYNC);
    cout << "initialization finished ......\n";

}

MyDB_BufferManager :: ~MyDB_BufferManager () {

    cout << "deconstruct MyDB_BufferManager, writing all data to disk\n";

    for(map<pair<MyDB_TablePtr, string>, MyDB_Page*>::iterator it=pinnedPageMap.begin(); 
            it!=pinnedPageMap.end(); ++it){
        writeToDisk(it->second, it->first.first->getStorageLoc());
    }
    for(map<pair<MyDB_TablePtr, string>, LRUNode *>::iterator it=LRUMap.begin();
            it!=LRUMap.end(); ++it){
        writeToDisk(it->second->pageLoc, it->first.first->getStorageLoc());
    }
    delete buffer;
    delete dummyHead;
    delete dummyTail;
    close(file);
}


MyDB_Page* MyDB_BufferManager :: loadPagefromDisk(string fileName, string pid) {
    const int fd = open(fileName.c_str(), O_RDONLY);
    if(fd == -1) return nullptr;
    MyDB_Page *tempPage = nullptr;
    int EOFLoc = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    do {
        //Read length first
        string temp;
        char b;
        while(read(fd, &b, 1)){
            if(b!=':')
                temp += b;
            else
                break;
        }
        size_t len = stoi(temp);
        //Read the content
        char *buf = new char[len + 1];
        //cout << "len:" << len << endl; 
        read(fd, buf, len);
        *(buf + len) = '\0';
        //cout << string(buf) << endl;
        tempPage = new MyDB_Page (string(buf));
        read(fd, &b, 1);
    } while(tempPage->getPageId().compare(pid) != 0 && EOFLoc > lseek(fd, 0, SEEK_CUR));
    cout<< "read page: " << tempPage->toString();

    close(fd);
    return tempPage;
}

void MyDB_BufferManager :: scanTempFile(MyDB_TablePtr tablePtr){
    const int fd = open(tablePtr->getStorageLoc().c_str(), O_RDONLY);
    int EOFLoc = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    cout << "EOFLoc " << EOFLoc << endl;
    if(fd == -1){
        return;
    }
    MyDB_Page *tempPage = nullptr;
    do {
        cout << "Now at: " << lseek(fd, 0, SEEK_CUR) << endl;
        //Read length first
        string temp;
        char b;
        while(read(fd, &b, 1)){
            if(b!=':')
                temp += b;
            else
                break;
        }
        size_t len = stoi(temp);
        //Read the content
        char *buf = new char[len + 1];
        //cout << "len:" << len << endl; 
        read(fd, buf, len);
        *(buf + len) = '\0';
        cout << "len: " << len << " buf: "<< buf << " size:" << string(buf).size() << endl;
        tempPage = new MyDB_Page (string(buf));
        cout << tempPage->toString() << " " << tempPage->getPageId() << endl;
        pair<MyDB_TablePtr, string> key = pair<MyDB_TablePtr, string>(tablePtr, tempPage->getPageId());
        pageStatus[key] = false;
        //Read the escape
        read(fd, &b, 1);
        //cout << "cur loc:" << lseek(fd, 0, SEEK_CUR) << endl;
    } while(EOFLoc > lseek(fd, 0, SEEK_CUR));

    //delete tempPage;
    close(fd);
}

void MyDB_BufferManager :: writeToDisk(MyDB_Page *page, string location){
    cout << "writeToDisk: "<<page->toString();
    MyDB_TablePtr tablePtr = pageTableMap[page];

    cout << "writing to " + location << endl;
    //File* outFile = fopen(location, "w+");
    //cout << page->toString() << endl; 
    const int fd = open(location.c_str(), O_RDWR | O_CREAT | O_FSYNC, 0666);
    int EOFLoc = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    MyDB_Page *tempPage = nullptr;
    if(lseek(fd, 0, SEEK_END) == 0){
        // empty file
        cout << "empty\n";
        write(fd, page->toString().c_str(), page->toString().size());
        close(fd);
        return;
    }

    lseek(fd, 0, SEEK_SET);
    bool flag = false;
    int prevPos = 0;
    int origPos = 0;
    do{
        prevPos = lseek(fd, 0, SEEK_CUR);
        string temp;
        char b;
        while(read(fd, &b, 1)){
            if(b!=':')
                temp += b;
            else
                break;
        }
        size_t len = stoi(temp);
        char *buf = new char[len + 1];
        //cout << "len:" << len << endl; 
        read(fd, buf, len);
        *(buf + len) = '\0';
        tempPage = new MyDB_Page (string(buf));
        read(fd, &b, 1);
        if(tempPage->getPageId().compare(page->getPageId()) == 0) {
            flag = true;
            origPos = lseek(fd, 0, SEEK_CUR);
            break;
        }
    }while(tempPage->getPageId().compare(page->getPageId()) != 0 && EOFLoc > lseek(fd, 0, SEEK_CUR));
    if(flag){
        lseek(fd, prevPos, SEEK_SET);
        cout << "prevPos: " << prevPos << " origPos: " << origPos << " size: " << page->toString().size() << endl;
    }
    write(fd, page->toString().c_str(), page->toString().size());
    close(fd);
    
    return;
}


#endif


