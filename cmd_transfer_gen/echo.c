#include <stdio.h>
#include <string.h>

#include "lwip/err.h"
#include "lwip/tcp.h"
#if defined (__arm__) || defined (__aarch64__)
#include "xil_printf.h"
#endif

#define MAX_ARG_N 31
#define MAX_RET_N 15
#include "xparameters.h"
typedef enum {DT_UINT8, DT_UINT16, DT_UINT32, DT_UINT8_P, DT_UINT16_P, DT_UINT32_P, DT_INT, DT_INT_P, DT_CHAR, DT_CHAR_P} DataType;

typedef struct {
	const char* funcName;
	uint8_t funcID;
	uint8_t num_args;
	uint8_t num_rets;
	DataType arg_types[MAX_ARG_N];
	DataType ret_types[MAX_RET_N];
} funcSpec;

typedef enum {
	FUNC_ID_AFE_SPI_RAW_WRITE = 0,
	FUNC_ID_AFE_SPI_RAW_READ,
	FUNC_ID_AFE_SPI_BURST_WRITE,
	FUNC_ID_AFE_SPI_BURST_READ,
	FUNC_ID_AFE_SPI_RAW_WRITE_MULTI,
	FUNC_ID_AFE_SPI_RAW_READ_MULTI,
	FUNC_ID_AFE_SPI_BURST_WRITE_MULTI,
	FUNC_ID_CHECK_FOR_UINT8PTR,
	FUNC_ID_CHECK_FOR_UINT16PTR,
	FUNC_ID_CHECK,
	FUNC_ID_CHECK_FOR_SPACE
} FunctionID;

funcSpec functions[] = {
	{"afeSpiRawWrite",  		 FUNC_ID_AFE_SPI_RAW_WRITE,     	3, 1, {DT_UINT8, DT_UINT16, DT_UINT8}, {DT_UINT32}},
	{"afeSpiRawRead",      	FUNC_ID_AFE_SPI_RAW_READ,      	3, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P}, {DT_UINT32}},
	{"afeSpiBurstWrite",   	FUNC_ID_AFE_SPI_BURST_WRITE,   	4, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P, DT_UINT16}, {DT_UINT32}},
	{"afeSpiBurstRead",    	FUNC_ID_AFE_SPI_BURST_READ,    	4, 1, {DT_UINT8, DT_UINT16, DT_UINT16, DT_UINT8_P}, {DT_UINT32}},
	{"afeSpiRawWriteMulti",	FUNC_ID_AFE_SPI_RAW_WRITE_MULTI,   3, 1, {DT_UINT8, DT_UINT16, DT_UINT8}, {DT_UINT32}},
	{"afeSpiRawReadMulti", 	FUNC_ID_AFE_SPI_RAW_READ_MULTI,	3, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P}, {DT_UINT32}},
	{"afeSpiBurstWriteMulti",  FUNC_ID_AFE_SPI_BURST_WRITE_MULTI, 4, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P, DT_UINT16}, {DT_UINT32}},
	{"func_check_for_uint8ptr",FUNC_ID_CHECK_FOR_UINT8PTR,    	1, 1, {DT_UINT8_P}, {0}},
	{"func_check_for_uint16ptr",FUNC_ID_CHECK_FOR_UINT16PTR,  	1, 1, {DT_UINT16_P}, {DT_UINT32}},
	{"func_check",         	FUNC_ID_CHECK,                 	8, 1, {DT_INT, DT_UINT8, DT_UINT16, DT_UINT32, DT_UINT8_P, DT_UINT16_P, DT_UINT32_P, DT_INT_P}, {DT_UINT32}},
	{"func_check_for_space",   FUNC_ID_CHECK_FOR_SPACE,       	2, 1, {DT_INT_P, DT_UINT8_P}, {0}}
};

/*
afeSpiRawWrite(2, 0x10F0, 0xF7)
afeSpiRawWriteMulti(10, 0x00E2, 192)
afeSpiRawRead(8, 0x2001, 0x0FFF0002)
afeSpiRawReadMulti(12, 0xF0DE, 0x0FFEE120)
afeSpiBurstWrite(4, 0x300F, {48, 192, 79, 127}, 4)
afeSpiBurstWriteMulti(9, 0x1940, {39, 182}, 2)
afeSpiBurstRead(1, 0x20F0, 4, 0x0FFFFEE0)
 */

typedef enum RET_TYPE
{
	TI_AFE_RET_EXEC_PASS = 0,
	TI_AFE_RET_EXEC_FAIL = 1
} RetType_e;

typedef struct {
	DataType type;
	union {
    	int32_t i32;
    	uint8_t u8;
    	uint16_t u16;
    	uint32_t u32;
    	struct {
        	void *ptr;
        	uint8_t len;
    	} arr;
	} val;
} DecodedArg;

//-------------------------------------------------------------------------
int decode_arg(DataType dt, uint8_t **pptr, int *premaining, DecodedArg *out) {
	uint8_t *ptr = *pptr;
	int remaining = *premaining;

	out->type = dt;

	switch(dt) {
    	case DT_INT: {
        	if (remaining < 1+4) return -1;
        	if (*ptr++ != DT_INT) return -1;
        	int32_t tmp;
        	memcpy(&tmp, ptr, 4);
        	out->val.i32 = ntohl(tmp);
        	ptr += 4; remaining -= 5;
    	} break;

    	case DT_UINT8: {
        	if (remaining < 1+1) return -1;
        	if (*ptr++ != DT_UINT8) return -1;
        	out->val.u8 = *ptr++;
        	remaining -= 2;
    	} break;

    	case DT_UINT16: {
        	if (remaining < 1+2) return -1;
        	if (*ptr++ != DT_UINT16) return -1;
        	uint16_t tmp;
        	memcpy(&tmp, ptr, 2);
        	out->val.u16 = ntohs(tmp);
        	ptr += 2; remaining -= 3;
    	} break;

    	case DT_UINT32: {
        	if (remaining < 1+4) return -1;
        	if (*ptr++ != DT_UINT32) return -1;
        	uint32_t tmp;
        	memcpy(&tmp, ptr, 4);
        	out->val.u32 = ntohl(tmp);
        	ptr += 4; remaining -= 5;
    	} break;

    	case DT_UINT8_P:
    	case DT_UINT16_P:
    	case DT_UINT32_P:
    	case DT_INT_P: {
        	if (remaining < 2) return -1;
        	if (*ptr++ != dt) return -1;
        	uint8_t mode = *ptr++;
        	remaining -= 2;

        	out->val.arr.len = 0;
        	out->val.arr.ptr = NULL;

        	if (mode & 0x80) {
            	// inline array
            	uint8_t len = mode & 0x7F;
            	int elem_size = (dt==DT_UINT8_P)?1 : (dt==DT_UINT16_P?2:4);
            	int need = len * elem_size;
            	if (remaining < need) return -1;
            	out->val.arr.len = len;
            	out->val.arr.ptr = ptr;

            	// endian-fix if >1 byte per element
            	if (elem_size == 2) {
                	uint16_t *arr = (uint16_t*)ptr;
                	for (int i=0;i<len;i++) arr[i] = ntohs(arr[i]);
            	} else if (dt == DT_UINT32_P && elem_size == 4) {
                	uint32_t *arr = (uint32_t*)ptr;
                	for (int i=0;i<len;i++) arr[i] = ntohl(arr[i]);
            	}
            	else if (dt == DT_INT_P && elem_size == 4) {
           		 int32_t *arr = (int32_t*)ptr;
           		 for (int i=0;i<len;i++) arr[i] = ntohl(arr[i]);
            	}
            	ptr += need; remaining -= need;
        	}
        	else {
            	// pointer by address
            	if (remaining < 4) return -1;
            	uint32_t tmp;
            	memcpy(&tmp, ptr, 4);
            	tmp = ntohl(tmp);
            	out->val.arr.len = 0;
            	out->val.arr.ptr = (void*)tmp;
            	ptr += 4; remaining -= 4;
        	}
    	} break;

    	default: return -1;
	}

	*pptr = ptr;
	*premaining = remaining;
	return 0;
}
//-------------------------------------------------------------------------
uint32_t afeSpiRawWrite(uint8_t afeInst, uint16_t addr, uint8_t data){
    printf("afeSpiRawWrite called:\n");

	printf("  afeInst (uint8)   = %02X\n", afeInst);
	printf("  addr (uint16)   = %04X\n", addr);
	printf("  data (uint8)  = %02X\n", data);
	return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiRawRead(uint8_t afeInst, uint16_t addr, uint8_t *readVal)
{
    printf("afeSpiRawRead called:\n");

	printf("  afeInst (uint8)   = %02X\n", afeInst);
	printf("  addr (uint16)   = %04X\n", addr);
    printf("  readVal points to the addr= %p\n", (void*)readVal);
	return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiBurstWrite(uint8_t afeInst, uint16_t addr, uint8_t *data, uint16_t dataArraySize)
{
    printf("afeSpiBurstWrite called:\n");

	printf("  afeInst (uint8)   = %02X\n", afeInst);
	printf("  addr (uint16)   = %04X\n", addr);
	printf("  dataArraySize (uint16)  = %u\n", dataArraySize);
    printf("  data (uint8[]): \n");
    for (int i = 0; i < dataArraySize; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
	return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiBurstRead(uint8_t afeInst, uint16_t addr, uint16_t dataArraySize, uint8_t *data)
{
	printf("afeSpiBurstRead called:\n");

	printf("  afeInst (uint8)   = %02X\n", afeInst);
	printf("  addr (uint16)   = %04X\n", addr);
	printf("  dataArraySize (uint16)  = %u\n", dataArraySize);
    printf("  Read data stored in the addr= %p\n", (void*)data);
    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiRawWriteMulti(uint8_t afeInstSel, uint16_t addr, uint8_t data)
{
    printf("afeSpiRawWriteMulti called:\n");

	printf("  afeInstSel (uint8)   = %02X\n", afeInstSel);
	printf("  addr (uint16)   = %04X\n", addr);
	printf("  data (uint8)  = %02X\n", data);
	return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiRawReadMulti(uint8_t afeInstSel, uint16_t addr, uint8_t *readVal)
{
    printf("afeSpiRawReadMulti called:\n");

	printf("  afeInstSel (uint8)   = %02X\n", afeInstSel);
	printf("  addr (uint16)   = %04X\n", addr);
    printf("  readVal points to the addr= %p\n", (void*)readVal);

	return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiBurstWriteMulti(uint8_t afeInstSel, uint16_t addr, uint8_t *data, uint16_t dataArraySize)
{
	printf("afeSpiBurstWriteMulti called:\n");

	printf("  afeInstSel (uint8)   = %02X\n", afeInstSel);
	printf("  addr (uint16)   = %04X\n", addr);
	printf("  dataArraySize (uint16)  = %u\n", dataArraySize);
    printf("  data (uint8[]): \n");
    for (int i = 0; i < dataArraySize; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
	return TI_AFE_RET_EXEC_PASS;
}

uint32_t func_check(
	int32_t arg1,
	uint8_t arg2,
	uint16_t arg3,
	uint32_t arg4,
	uint8_t *arg5, uint8_t arg5_len,
	uint16_t *arg6, uint8_t arg6_len,
	uint32_t *arg7, uint8_t arg7_len,
	int32_t *arg8, uint8_t arg8_len
)
{
	printf("func_check called:\n");

	printf("  arg1 (int32)   = %ld\n", arg1);
	printf("  arg2 (uint8)   = %u\n", arg2);
	printf("  arg3 (uint16)  = %u\n", arg3);
	printf("  arg4 (uint32)  = %lu\n", arg4);

	// arg5 (uint8 array / pointer)
	if (arg5_len) {
    	printf("  arg5 (uint8[]) len=%u : ", arg5_len);
    	for (int i = 0; i < arg5_len; i++) {
        	printf("%02X ", arg5[i]);
    	}
    	printf("\n");
	} else {
    	printf("  arg5 pointer to addr=%p\n", (void*)arg5);
	}

	// arg6 (uint16 array / pointer)
	if (arg6_len) {
    	printf("  arg6 (uint16[]) len=%u : ", arg6_len);
    	for (int i = 0; i < arg6_len; i++) {
        	printf("%04X ", arg6[i]);
    	}
    	printf("\n");
	} else {
    	printf("  arg6 pointer to addr=%p\n", (void*)arg6);
	}

	// arg7 (uint32 array / pointer)
	if (arg7_len) {
    	printf("  arg7 (uint32[]) len=%u : ", arg7_len);
    	for (int i = 0; i < arg7_len; i++) {
        	printf("%08luX ", arg7[i]);
    	}
    	printf("\n");
	} else {
    	printf("  arg7 pointer to addr=%p\n", (void*)arg7);
	}

	// arg8 (int32 array / pointer)
	if (arg8_len) {
    	printf("  arg8 (int32[]) len=%u : ", arg8_len);
    	for (int i = 0; i < arg8_len; i++) {
        	printf("%ld ", arg8[i]);
    	}
    	printf("\n");
	} else {
    	printf("  arg8 pointer to addr=%p\n", (void*)arg8);
	}
	return TI_AFE_RET_EXEC_PASS;
}

uint32_t func_check_for_uint16ptr(
	uint16_t *arg1, uint8_t arg1_len
)
{
	printf("func_check_for_uint16ptr called:\n");

	if (arg1_len) {
    	printf("  arg1 (uint16[]) len=%u : ", arg1_len);
    	for (int i = 0; i < arg1_len; i++) {
        	printf("%04X ", arg1[i]);
    	}
    	printf("\n");
	} else {
    	printf("  arg1 pointer to addr=%p\n", (void*)arg1);
	}
	return TI_AFE_RET_EXEC_PASS;
}

uint32_t func_check_for_uint8ptr(
	uint8_t *arg1, uint8_t arg1_len
)
{
	printf("func_check_for_uint8ptr called:\n");

	if (arg1_len) {
    	printf("  arg1 (uint8[]) len=%u : ", arg1_len);
    	for (int i = 0; i < arg1_len; i++) {
        	printf("%02X ", arg1[i]);
    	}
    	printf("\n");
	} else {
    	printf("  arg1 pointer to addr=%p\n", (void*)arg1);
	}
	return TI_AFE_RET_EXEC_PASS;
}
//----------------------------------------------------------------------------
int transfer_data() {
    return 0;
}

void print_app_header()
{
#if (LWIP_IPV6==0)
    xil_printf("\n\r\n\r-----lwIP TCP echo server ------\n\r");
#else
    xil_printf("\n\r\n\r-----lwIPv6 TCP echo server ------\n\r");
#endif
    xil_printf("TCP packets sent to port 6001 will be echoed back\n\r");
}

err_t recv_callback(void *arg, struct tcp_pcb *tpcb,
                           	struct pbuf *p, err_t err)
{
    /* do not read the packet if we are not in ESTABLISHED state */
    if (!p) {
   	 tcp_close(tpcb);
   	 tcp_recv(tpcb, NULL);
   	 return ERR_OK;
    }

    /* indicate that the packet has been received */
    tcp_recved(tpcb, p->len);

    uint8_t* buf = malloc(p->tot_len);
    if (!buf){
    	xil_printf("malloc failed\n\r");
    	pbuf_free(p);
    	return ERR_MEM;
    }
    pbuf_copy_partial(p, buf, p->tot_len, 0);
    size_t len = p->tot_len;

/*	if (len<2){
    	xil_printf("Invalid packet \n\r");
    	pbuf_free(p);
    	return ERR_OK;
	}
	*/

	uint8_t cmd_id = buf[0];
	uint8_t num_args = buf[1]>>3;
	uint8_t num_ret = buf[1] & (uint8_t)7;
	uint8_t *ptr = buf + 2;
	size_t remaining = len - 2;
	uint32_t ret;
	uint8_t status;
	int ok;
	switch(cmd_id){
    	case FUNC_ID_AFE_SPI_RAW_WRITE:
    	{
        	funcSpec *spec = &functions[0]; // index for func_check
        	if (num_args != spec->num_args) {
            	//xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = afeSpiRawWrite(
            	decoded[0].val.u8,
            	decoded[1].val.u16,
            	decoded[2].val.u8);
            status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);
    	break;
    	}

    	case FUNC_ID_AFE_SPI_RAW_READ:
    	{
        	funcSpec *spec = &functions[1]; // index for func_check
        	if (num_args != spec->num_args) {
            	xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = afeSpiRawRead(
            	decoded[0].val.u8,
            	decoded[1].val.u16,
            	(uint8_t*)decoded[2].val.arr.ptr
        	);
            status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);

    	break;
    	}

    	case FUNC_ID_AFE_SPI_BURST_WRITE:
    	{
        	funcSpec *spec = &functions[2]; // index for func_check
        	if (num_args != spec->num_args) {
            	xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = afeSpiBurstWrite(
            	decoded[0].val.u8,
            	decoded[1].val.u16,
            	(uint8_t*)decoded[2].val.arr.ptr,
            	decoded[3].val.u16
        	);
        	status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);

    	break;
    	}

    	case FUNC_ID_AFE_SPI_BURST_READ:
    	{
        	funcSpec *spec = &functions[3];
        	if (num_args != spec->num_args) {
            	xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = afeSpiBurstRead(
            	decoded[0].val.u8,
            	decoded[1].val.u16,
            	decoded[2].val.u16,
            	(uint8_t*)decoded[3].val.arr.ptr
        	);
        	status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);

    	break;
    	}

    	case FUNC_ID_AFE_SPI_RAW_WRITE_MULTI:
    	{
        	funcSpec *spec = &functions[4]; // index for func_check
        	if (num_args != spec->num_args) {
            	xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = afeSpiRawWriteMulti(
            	decoded[0].val.u8,
            	decoded[1].val.u16,
            	decoded[2].val.u8
        	);
        	status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);

    	break;
    	}

    	case FUNC_ID_AFE_SPI_RAW_READ_MULTI:
    	{
        	funcSpec *spec = &functions[5]; // index for func_check
        	if (num_args != spec->num_args) {
            	xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = afeSpiRawReadMulti(
            	decoded[0].val.u8,
            	decoded[1].val.u16,
            	decoded[2].val.arr.ptr
        	);
        	status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);

    	break;
    	}

    	case FUNC_ID_AFE_SPI_BURST_WRITE_MULTI:
    	{
        	funcSpec *spec = &functions[6]; // index for func_check
        	if (num_args != spec->num_args) {
            	xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = afeSpiBurstWriteMulti(
            	decoded[0].val.u8,
            	decoded[1].val.u16,
            	decoded[2].val.arr.ptr,
            	decoded[3].val.u16
        	);
        	status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);

    	break;
    	}

    	case FUNC_ID_CHECK_FOR_UINT8PTR:
    	{
        	funcSpec *spec = &functions[7]; // index for func_check
        	if (num_args != spec->num_args) {
            	xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = func_check_for_uint8ptr(
            	decoded[0].val.arr.ptr, decoded[0].val.arr.len
        	);
        	status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);

    	break;
    	}

    	case FUNC_ID_CHECK_FOR_UINT16PTR:
    	{
        	funcSpec *spec = &functions[8]; // index for func_check
        	if (num_args != spec->num_args) {
            	xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = func_check_for_uint16ptr(
            	decoded[0].val.arr.ptr, decoded[0].val.arr.len
        	);
        	status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);

    	break;
    	}

    	case FUNC_ID_CHECK:
     	{
        	funcSpec *spec = &functions[9]; // index for func_check
        	if (num_args != spec->num_args) {
            	xil_printf("%s: wrong arg count %d\n\r", spec->funcName, num_args);
            	break;
        	}

        	DecodedArg decoded[MAX_ARG_N];
        	uint8_t *p = ptr;
        	int rem = remaining;

        	ok = 1;
        	for (int i=0; i<spec->num_args; i++) {
            	if (decode_arg(spec->arg_types[i], &p, &rem, &decoded[i]) < 0) {
                	xil_printf("arg %d decode failed\n\r", i);
                	ok = 0;
                	break;
            	}
        	}
        	if (!ok) break;

        	ret = func_check(
            	decoded[0].val.i32,
            	decoded[1].val.u8,
            	decoded[2].val.u16,
            	decoded[3].val.u32,
            	decoded[4].val.arr.ptr, decoded[4].val.arr.len,
            	decoded[5].val.arr.ptr, decoded[5].val.arr.len,
            	decoded[6].val.arr.ptr, decoded[6].val.arr.len,
            	decoded[7].val.arr.ptr, decoded[7].val.arr.len
        	);
        	status = (uint8_t)ret;
            err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
            if (werr != ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
            tcp_output(tpcb);

        	break;
    	}

    	case FUNC_ID_CHECK_FOR_SPACE:
    	break;
	}
	free(buf);
	pbuf_free(p);
	return ERR_OK;
}


err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    static int connection = 1;

    /* set the receive callback for this connection */
    tcp_recv(newpcb, recv_callback);

    /* just use an integer number indicating the connection id as the
   	callback argument */
    tcp_arg(newpcb, (void*)(UINTPTR)connection);

    /* increment for subsequent accepted connections */
    connection++;

    return ERR_OK;
}

int start_application()
{
    struct tcp_pcb *pcb;
    err_t err;
    unsigned port = 7;

    /* create new TCP PCB structure */
    pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
   	 xil_printf("Error creating PCB. Out of Memory\n\r");
   	 return -1;
    }

    /* bind to specified @port */
    err = tcp_bind(pcb, IP_ANY_TYPE, port);
    if (err != ERR_OK) {
   	 xil_printf("Unable to bind to port %d: err = %d\n\r", port, err);
   	 return -2;
    }

    /* we do not need any arguments to callback functions */
    tcp_arg(pcb, NULL);

    /* listen for connections */
    pcb = tcp_listen(pcb);
    if (!pcb) {
   	 xil_printf("Out of memory while tcp_listen\n\r");
   	 return -3;
    }

    /* specify callback to use for incoming connections */
    tcp_accept(pcb, accept_callback);
    xil_printf("TCP echo server started @ port %d\n\r", port);
    return 0;
}
