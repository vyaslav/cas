#pragma once
#include "windows.h"

#define SAFE_LONGS 2

#define BANK_IS_FREE 0
#define BANK_IS_READ 1
#define EOWS 3
#define CTRL_VALS 3


class CASBuffer{
public:
	//run from main thread
	CASBuffer(unsigned int size):buf(new long[size+SAFE_LONGS]), buf_size(size+SAFE_LONGS),read_pos(0), write_pos(0)
	{
		memset((void*)buf, 0, buf_size*sizeof(long));
	}

	~CASBuffer(){
		delete[] buf;
	}
	//run from producer thread
	void put(const char* in_buf, unsigned int bytes);
	//run from consumer thread
	unsigned int CASBuffer::get(char* in_buf);
	//run from producer thread
	void close(){
		put(EOWS);
	};
	//run from main thread
	void reset(){
		read_pos = write_pos = 0;
		memset((void*)buf, 0, buf_size*sizeof(long));
	}

private:
	long volatile * const buf;

	const unsigned int buf_size;

	unsigned int read_pos;
	unsigned int write_pos;

	void skip_write(){
		write_pos=(write_pos+1>=buf_size)?0:write_pos+1;
	}

	void skip_read(){
		read_pos=(read_pos+1>=buf_size)?0:read_pos+1;
	}

	long CASBuffer::get(bool isCtrl=false, long xch = BANK_IS_FREE);
	void put(long val);
};