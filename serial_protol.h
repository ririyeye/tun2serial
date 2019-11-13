#ifndef __serial_protol_h_____
#define __serial_protol_h_____

struct ser_data {
	unsigned char * buff;
	int len;
};

#ifdef __cplusplus 
extern "C" {
#endif




	int serialEncode(struct ser_data * src, struct ser_data * dst, int seq);
	int serialFindDecode(struct ser_data * src, struct ser_data * dst, int dstsz);


#ifdef __cplusplus 
}
#endif


#endif
