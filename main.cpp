#include "CASBuffer.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

CASBuffer buf(1024);
char x[1024];


DWORD WINAPI
ProducerThread(
    LPVOID unused
    )
{
	for(int i=0;i<2000000;i++){
		buf.put(x, 1024);
	}
	buf.close();
 
    return 0;
}
 
DWORD WINAPI
ConsumerThread(
    LPVOID unused
    )
{
	clock_t start = clock();
    int read = 0;
	char y[1024];
	memset(y,0,1024);

	for(int i=0;(read = buf.get(y));i++){
		//cout<<"["<<i<<"]";		
		//y[26]=200;
		if(read!=1024)
			cout<<"Alarm! read is not 1024|"<<read;
		for(int j=0;j<1024;j++)
			if(x[j]!=y[j])
				cout<<"Alarm! "<<j<<" x!=y|"<<(int)(x[j]&0xFF)<<"/"<<(int)(y[j]&0xFF)<<endl;

		memset(y,0,1024);

	}
	double diff = ( clock() - start ) / (double)CLOCKS_PER_SEC;
	cout << "Read in "<<diff<<" sec";
    return 0;
}

int main() {
	srand ( time(NULL) );
	for(int i=0;i<1024;i++)
		x[i]= rand() % 100 + 1;

//threads
    int unused1, unused2;
    HANDLE threads[2];

    threads[0] = CreateThread(NULL,
                              0,
                              ProducerThread,
                              &unused1,
                              0,
                              (LPDWORD)&unused2);
 
    threads[1] = CreateThread(NULL,
                              0,
                              ConsumerThread,
                              &unused1,
                              0,
                              (LPDWORD)&unused2);

WaitForMultipleObjects(2, threads, TRUE, INFINITE);

cout<<"end"<<endl;
getchar();

}