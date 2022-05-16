#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

int sum=0;

int atomic_flag=1;
pthread_mutex_t mlock=PTHREAD_MUTEX_INITIALIZER;
void slock(pthread_mutex_t *l){
	int res=pthread_mutex_lock(l);
	assert(res==0);
}
void unlock(pthread_mutex_t *l) {
	int res=pthread_mutex_unlock(l);
	assert(res==0);
}
int atomic_CAS(int *addr,int expected,int new_value) {
	asm volatile("lock cmpxchg %[new],%[ptr]"
			:"+a"(expected),[ptr] "+m"(*addr)
			:[new] "r"(new_value)
			:"memory");
	return expected;

}
void * Tsum()
{
    for(int i=0;i<100000;i++)
	    sum++;
    return NULL;
}

void *Tsum_mutex()
{
	slock(&mlock);
	Tsum();
	unlock(&mlock);
	return NULL;
}
void *Tsum_atomic()
{
	while(atomic_CAS(&atomic_flag,1,0)==0);
	Tsum();
	atomic_CAS(&atomic_flag,0,1);
	return NULL;
}
int main(void)
{
    pthread_t TA,TB;
    printf("None\nexpect 200000\n===============\n");
    pthread_create(&TA,NULL,Tsum,NULL);
    pthread_create(&TB,NULL,Tsum,NULL);
    pthread_join(TA,NULL);
    pthread_join(TB,NULL);
    printf("%d\n===============\n",sum);
    sum=0;
    printf("mutex\nexpect 200000\n===============\n");
    pthread_create(&TA,NULL,Tsum_mutex,NULL);
    pthread_create(&TB,NULL,Tsum_mutex,NULL);
    pthread_join(TA,NULL);
    pthread_join(TB,NULL);
    printf("%d\n===============\n",sum);
    sum=0;
    printf("atomic\nexpect 200000\n===============\n");
    pthread_create(&TA,NULL,Tsum_atomic,NULL);
    pthread_create(&TB,NULL,Tsum_atomic,NULL);
    pthread_join(TA,NULL);
    pthread_join(TB,NULL);
    printf("%d\n===============\n",sum);
    return 0;
}
