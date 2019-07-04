#ifndef __serial_protol_h___
#define __serial_protol_h___


#define HEAD_NUM (0x08) 
#define TAIL_NUM (0x07)

#ifdef __cplusplus 
extern "C" {
#endif

struct ser_data{
	unsigned char * buff;
	int len;
};


int serialEncode(struct ser_data * src, struct ser_data * dst);
int serialFindDecode(struct ser_data * src , struct ser_data * dst);


#ifdef __cplusplus 
}
#endif


#endif
