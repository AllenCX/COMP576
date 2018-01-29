
#ifndef CATALOG_UNIT_H
#define CATALOG_UNIT_H

#include "MyDB_BufferManager.h"
#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"
#include "QUnit.h"
#include <iostream>
#include <unistd.h>
#include <vector>

using namespace std;

// these functions write a bunch of characters to a string... used to produce data
void writeNums (char *bytes, size_t len, int i) {
	for (size_t j = 0; j < len - 1; j++) {
		bytes[j] = '0' + (i % 10);
	}
	bytes[len - 1] = 0;
}

void writeLetters (char *bytes, size_t len, int i) {
	for (size_t j = 0; j < len - 1; j++) {
		bytes[j] = 'i' + (i % 10);
	}
	bytes[len - 1] = 0;
}

void writeSymbols (char *bytes, size_t len, int i) {
	for (size_t j = 0; j < len - 1; j++) {
		bytes[j] = '!' + (i % 10);
	}
	bytes[len - 1] = 0;
}

int main () {

	QUnit::UnitTest qunit(cerr, QUnit::verbose);
	// UNIT TEST 1: A BIG ONE!!
	/*
	{

		// create a buffer manager 
		MyDB_BufferManager myMgr (64, 16, "tempDSFSD");
		MyDB_TablePtr table1 = make_shared <MyDB_Table> ("tempTable", "foobar");

		// allocate a pinned page
		cout << "allocating pinned page\n";
		MyDB_PageHandle pinnedPage = myMgr.getPinnedPage (table1, 0);
		char *bytes = (char *) pinnedPage->getBytes ();
		writeNums (bytes, 64, 0);
		pinnedPage->wroteBytes ();
		cout << pinnedPage->getPagePtr()->toString() << endl;
		
		// create a bunch of pinned pages and remember them
		vector <MyDB_PageHandle> myHandles;
		for (int i = 1; i < 10; i++) {
			cout << "allocating pinned page\n";
			MyDB_PageHandle temp = myMgr.getPinnedPage (table1, i);
			char *bytes = (char *) temp->getBytes ();
			writeNums (bytes, 64, i);
			temp->wroteBytes ();
			myHandles.push_back (temp);
			cout << temp->getPagePtr()->toString() << endl;
		}
		
		// now forget the pages we created
		vector <MyDB_PageHandle> temp;
		myHandles = temp;

		// now remember 8 more pages
		for (int i = 0; i < 8; i++) {
			cout << "allocating pinned page\n";
			MyDB_PageHandle temp = myMgr.getPinnedPage (table1, i);
			char *bytes = (char *) temp->getBytes ();

			// write numbers at the 0th position
			if (i == 0)
				writeNums (bytes, 64, i);
			else
				writeSymbols (bytes, 64, i);
			temp->wroteBytes ();
			myHandles.push_back (temp);
			cout << "temp in QUnit: "<<temp->getPagePtr()->toString() << endl;
		}
		cout << "=======================================================================" << endl;
		
		// now correctly write nums at the 0th position
		cout << "allocating unpinned page\n";
		MyDB_PageHandle anotherDude = myMgr.getPage (table1, 0);
		bytes = (char *) anotherDude->getBytes ();
		writeSymbols (bytes, 64, 0);
		anotherDude->wroteBytes ();
		cout << anotherDude->getPagePtr()->toString() << endl;
		// now do 90 more pages, for which we forget the handle immediately
		for (int i = 10; i < 100; i++) {
			cout << "allocating unpinned page\n";
			MyDB_PageHandle temp = myMgr.getPage (table1, i);
			char *bytes = (char *) temp->getBytes ();
			writeNums (bytes, 64, i);
			temp->wroteBytes ();
			cout << temp->getPagePtr()->toString() << endl;
		}
		cout << "=======================================================================" << endl;
		// now forget all of the pinned pages we were remembering
		vector <MyDB_PageHandle> temp2;
		myHandles = temp2;

		// now get a pair of pages and write them
		for (int i = 0; i < 100; i++) {
			cout << "allocating pinned page\n";
			MyDB_PageHandle oneHandle = myMgr.getPinnedPage ();
			char *bytes = (char *) oneHandle->getBytes ();
			writeNums (bytes, 64, i);
			oneHandle->wroteBytes ();
			cout << i << ": "<< oneHandle->getPagePtr()->toString() << endl;

			cout << "allocating pinned page\n";
			MyDB_PageHandle twoHandle = myMgr.getPinnedPage ();
			writeNums (bytes, 64, i);
			twoHandle->wroteBytes ();
			cout << i << ": "<<twoHandle->getPagePtr()->toString() << endl;
		}
		cout << "=======================================================================" << endl;
		
		
		// make a second table
		MyDB_TablePtr table2 = make_shared <MyDB_Table> ("tempTable2", "barfoo");
		for (int i = 0; i < 100; i++) {
			cout << "allocating unpinned page\n";
			MyDB_PageHandle temp = myMgr.getPage (table2, i);
			char *bytes = (char *) temp->getBytes ();
			writeLetters (bytes, 64, i);
			temp->wroteBytes ();
			cout<< "i:" <<i << " " << temp->getPagePtr()->toString() << endl;
		}
		
	}
	
	*/
	
	// UNIT TEST 2
	{
		MyDB_BufferManager myMgr (64, 16, "tempDSFSD");
		MyDB_TablePtr table1 = make_shared <MyDB_Table> ("tempTable", "foobar");

		// look up all of the pages, and make sure they have the correct numbers
		for (int i = 0; i < 100; i++) {
			MyDB_PageHandle temp = myMgr.getPage (table1, i);
			char answer[64];
			if (i < 8)
				writeSymbols (answer, 64, i);
			else
				writeNums (answer, 64, i);
			char *bytes = (char *) temp->getBytes ();
			//cout << "UnitTest: " << temp->getPagePtr()->toString() << endl;
			QUNIT_IS_EQUAL (string (answer), string (bytes));
		}
		
	}
	
}

#endif
