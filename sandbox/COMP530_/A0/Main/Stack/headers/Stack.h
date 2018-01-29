
#ifndef STACK_H
#define STACK_H

// this is the node class used to build up the LIFO stack
template <class Data>
class Node {

private:

	Data holdMe;
	Node *next;
	
public:
	/*****************************************/
	/** WHATEVER CODE YOU NEED TO ADD HERE!! */
	/*****************************************/
    Node(Data data){
        holdMe = data;
        next = nullptr;
    }
    Node(){
        next = nullptr;
    }
    ~Node(){
        next = nullptr;
    }
    
    Data getData(){
        return holdMe;
    }
    
    void setData(Data data){
        holdMe = data;
    }
    Node* getNext(){
        return next;
    }
    
    void setNext(Node *nextNode){
        next = nextNode;
    }
};

// a simple LIFO stack
template <class Data> 
class Stack {

	Node <Data> *head;

public:

	// destroys the stack
	~Stack () {
        // pop/delete all the nodes in the stack
        while(!isEmpty()){
            pop();
        }
        if(head){
            //std::cout << "delete head \n";
            delete head;
        }
    }

	// creates an empty stack
    Stack () {
        head = new Node<Data>(); // dummy head
    }

	// adds pushMe to the top of the stack
	void push (Data data) {
        Node<Data> *p = head->getNext();
        Node<Data> *newNode = new Node<Data>(data);
        
        head->setNext(newNode);
        newNode->setNext(p);
    }

	// return true if there are not any items in the stack
    bool isEmpty () { return head->getNext() == nullptr; }

	// pops the item on the top of the stack off, returning it...
	// if the stack is empty, the behavior is undefined
	Data pop () {
        if(!isEmpty()){
            Node<Data> *p = head->getNext();
            head->setNext(p->getNext());
            Data data = p->getData();
            delete p;
            //std::cout<< "pop " << data << "\n";
            return data;
        }else{
            //std::cout<< "empty\n";
            return head->getData();
        }
    }
};

#endif
