#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
//public part
typedef struct Node{
    struct Node * next;
    int value;
}Node;
//mutex_lock implementation
//
pthread_mutex_t mlock=PTHREAD_MUTEX_INITIALIZER;

void slock(pthread_mutex_t * l)
{
     int res=pthread_mutex_lock(l);
     assert(res==0);
}
void unlock(pthread_mutex_t * l)
{
     int res=pthread_mutex_unlock(l);
     assert(res==0);
}
void push_mutex(Node **top_ptr, Node * n) {
	slock(&mlock);
	n->next =*top_ptr;
	*top_ptr=n;
	unlock(&mlock);
}
Node * pop_mutex(Node **top_ptr)
{
	slock(&mlock);
	if(*top_ptr==NULL)
		return NULL;
	Node *p=*top_ptr;
	*top_ptr=*top_ptr->next;
	unlock(&mlock);
	return p;
}
//atomic operation implemention
int flag=1;
int atomic_CAS(int *addr,int expected, int new_value)
{
	asm volatile("lock cmpxchg %[new],%[ptr]"
			:"+a"(expected),[ptr] "+m"(*addr)
			:[new] "r"(new_value)
			:"memory");
	return expected;
}
void push_atomic(Node **top_ptr,Node * n) {
	while(atomic_CAS(&flag,1,0)==0);
	n->next=*top_ptr;
	*top_ptr=n;
	atomic_CAS(&flag,0,1);
}
Node * pop_atomic(Node** top_ptr) {
	while(atomic_CAS(&flag,1,0)==0);
	if(*top_ptr==NULL)
		return NULL;
	Node *p=*top_ptr;
	*top_ptr=*top_ptr->next;
	atomic_CAS(&flag,0,1);
	return p;
}

