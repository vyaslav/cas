#include "CASBuffer.h"

void CASBuffer::put(long val){
	long i=0;
	bool isFirst = true;

	while(
		InterlockedCompareExchange(&buf[write_pos], 
                                  val, BANK_IS_FREE) != BANK_IS_FREE
		);

	skip_write();
}

void CASBuffer::put(const char* in_buf, unsigned int bytes){
	//convert to long
	unsigned int longs = bytes/sizeof(long);
	long rest = 0;
	memcpy(&rest, in_buf+(bytes-bytes%sizeof(long)),bytes%sizeof(long)); 
	/*for(int i=0; i<bytes%sizeof(long); i++){
		rest|=in_buf[bytes-(i+1)];
		rest<<=sizeof(char)*8;
	}*/

	long * in_lBuf = (long*)in_buf;

	unsigned int ctrl_pos = write_pos;
	//wait until i write
	put(BANK_IS_FREE);
	for(int i=0;i<longs;i++){
		put(in_lBuf[i]);
	}
	put(rest);	
	unsigned int this_pos = write_pos;
	write_pos=ctrl_pos;
	put(bytes+CTRL_VALS);
	write_pos=this_pos;
}

long CASBuffer::get(bool isCtrl, long xch){
	long xchng = BANK_IS_FREE;
	long result=BANK_IS_FREE;
	long i, hit_cnt=0;
	bool isFirst = true;
    for (;;) {
		i = InterlockedCompareExchange(&buf[read_pos], 
                                  xchng, 
								  result);
		if(isCtrl && isFirst){
			if(i != BANK_IS_FREE){
				xchng = xch;
			}else{
				if(++hit_cnt==10){
					hit_cnt=0;
					Sleep(0.1);
				}
				continue;
			}
		}

		if(isFirst)
			result = i;

		if(!isFirst)
			break;

		isFirst=false;
    }
	skip_read();
	return result;
}

unsigned int CASBuffer::get(char* in_buf){
	unsigned int ctrl_pos = read_pos;
	unsigned int ctrl = get(true, BANK_IS_READ);

	if(ctrl==EOWS){
		//restore state
		read_pos=ctrl_pos;
		get(true, EOWS);
		read_pos=ctrl_pos;
		return 0;
	}
	unsigned int bytes = ctrl-CTRL_VALS;
	unsigned int longs = bytes/sizeof(long);
	long rest = 0;

	long * in_lBuf = (long*)in_buf;
	for(int i=0;i<longs;i++){
		in_lBuf[i]=get();
	}
	rest = get();
	memcpy(in_buf+(bytes-bytes%sizeof(long)), &rest,bytes%sizeof(long)); 
	/*
	for(int i=bytes%sizeof(long); i>0; i--){
		rest>>=sizeof(char)*8;
		in_buf[bytes-i]=rest&0xFF;
	}*/

	unsigned int this_pos = read_pos;
	read_pos=ctrl_pos;
	get();
	read_pos=this_pos;

	return bytes;
}