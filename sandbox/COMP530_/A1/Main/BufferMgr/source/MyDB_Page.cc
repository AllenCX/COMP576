
#ifndef PAGE_C
#define PAGE_C

#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <string.h>
#include "MyDB_Page.h"

using namespace std;

MyDB_Page ::MyDB_Page(){
    // Do nothing
}

MyDB_Page :: MyDB_Page(string id, size_t pz, string name, bool pin, int tp) {
    pageId = id;
    pageSize = pz;
    pinned = pin;
    type = tp;
    handleCnt = 0;
    pageData = new char[pz];
    memset(pageData, 'z', pz-1);
    pageData[pz - 1] = 0;
    writeFlag = false;
    dbName = name;
}

MyDB_Page :: MyDB_Page(string s){
    vector <string> ret;
    stringstream ss(s);
    string temp;
    while(getline(ss, temp, '|')) {
        *(back_inserter(ret)++) = temp;
    }
    //cout << ret[0] << endl;
    pageId = ret[0];
    pageSize = stoi(ret[1]);
    type = stoi(ret[2]);
    pinned = bool(stoi(ret[3]));
    handleCnt = stoi(ret[4]);
    const char * tempData = ret[5].c_str();
    pageData = new char[strlen(tempData)];
    memcpy(pageData, tempData, strlen(tempData));
    if (type == 1){
        dbName = ret[6];
        //dbPageId = stoi(ret[7]);
    }
    writeFlag = bool(stoi(ret[7]));
}

MyDB_Page :: ~MyDB_Page() {
}

void MyDB_Page :: writeData(char* pd) {
    memcpy(pageData, pd, pageSize);
}

int MyDB_Page :: getType() {
    return type;
}

string MyDB_Page :: getPageId() {
    return pageId;
}

void MyDB_Page :: addHandleCnt() {
    handleCnt++;
}

int MyDB_Page :: subHandleCnt() {
    handleCnt--;
    if(handleCnt)
        return 0;
    if(type == 1 && pinned == true && handleCnt == 0) {
        pinned = false;
        //cout << "unpin page "<< pageId <<"count " << handleCnt << endl;
        return 1;
    }
    if(type == 0 && handleCnt == 0) {
        //this->~MyDB_Page();
        pinned = false;
        return -1;
    }
    return 2;
}

bool MyDB_Page :: isThisDBPage(string name, long i) {
    if(dbName == name && dbPageId == i)
        return true;
    return false;
}

void MyDB_Page :: setDBInfo(string name, long i) {
    dbName = name;
    dbPageId = i;
}


char * MyDB_Page :: getBytes() {
    return pageData;
}

void MyDB_Page :: setWritten(bool flag){
    writeFlag = flag;
}

bool MyDB_Page :: isWritten(){
    return writeFlag;
}

string MyDB_Page :: toString() {
    string ret;
    ret += pageId + '|';
    ret += to_string(pageSize) + '|';
    ret += to_string(type) + '|';
    ret += to_string(pinned) + '|';
    ret += to_string(handleCnt) + '|';
    ret += string(pageData) + '|';
    if(type == 1){
        ret += dbName + '|';
        //ret += to_string(dbPageId);
    }
    ret += to_string(writeFlag);
    
    ret = to_string(ret.size()) + ":" + ret + "\n";
    return ret;
}

bool MyDB_Page :: pin(){
    pinned = true;
    return pinned;
}

bool MyDB_Page :: unpin(){
    pinned = false;
    return pinned;
}
#endif

