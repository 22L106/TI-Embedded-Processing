#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lwip/err.h"
#include "lwip/tcp.h"
#include "xil_printf.h"
#include <stdbool.h>
#define MAX_ARG_N 31
#define MAX_RET_N 15

int transfer_data() {
    return 0;
}

void print_app_header()
{
#if (LWIP_IPV6==0)
    xil_printf("\n\r\n\r-----lwIP TCP server ------\n\r");
#else
    xil_printf("\n\r\n\r-----lwIPv6 TCP server ------\n\r");
#endif
}

typedef uint32_t (*PFnApiCall)(void **args); //PFnApiCall is the pointer that can point to functions with void **args argument and uint32_t return type

typedef enum {
    	DT_UINT8, DT_UINT16, DT_UINT32, DT_INT32, DT_UINT8_P, DT_UINT16_P, DT_UINT32_P, DT_INT32_P, DT_CHAR, DT_CHAR_P} DataType;

typedef struct {
    PFnApiCall funcName;	//function name without paranthesis is pointer to that function (address to the function's entry point)
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
    FUNC_ID_AFE_SPI_BURST_WRITE_MULTI
} FunctionID;

typedef enum RET_TYPE
{
    TI_AFE_RET_EXEC_PASS = 0,
    TI_AFE_RET_EXEC_FAIL = 1
} RetType_e;

uint32_t afeSpiRawWrite(void** args);
uint32_t afeSpiRawRead(void** args);
uint32_t afeSpiBurstWrite(void** args);
uint32_t afeSpiBurstRead(void** args);
uint32_t afeSpiRawWriteMulti(void** args);
uint32_t afeSpiRawReadMulti(void** args);
uint32_t afeSpiBurstWriteMulti(void** args);

funcSpec funcTable[] = {
    {afeSpiRawWrite, 		  FUNC_ID_AFE_SPI_RAW_WRITE,		 3, 1, {DT_UINT8, DT_UINT16, DT_UINT8}, {DT_UINT32}},
    {afeSpiRawRead, 		 FUNC_ID_AFE_SPI_RAW_READ, 		 3, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P}, {DT_UINT32}},
    {afeSpiBurstWrite,  	 FUNC_ID_AFE_SPI_BURST_WRITE,  	 4, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P, DT_UINT16}, {DT_UINT32}},
    {afeSpiBurstRead,   	 FUNC_ID_AFE_SPI_BURST_READ,   	 4, 1, {DT_UINT8, DT_UINT16, DT_UINT16, DT_UINT8_P}, {DT_UINT32}},
    {afeSpiRawWriteMulti,    FUNC_ID_AFE_SPI_RAW_WRITE_MULTI,   3, 1, {DT_UINT8, DT_UINT16, DT_UINT8}, {DT_UINT32}},
    {afeSpiRawReadMulti,     FUNC_ID_AFE_SPI_RAW_READ_MULTI,    3, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P}, {DT_UINT32}},
    {afeSpiBurstWriteMulti,  FUNC_ID_AFE_SPI_BURST_WRITE_MULTI, 4, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P, DT_UINT16}, {DT_UINT32}},
};

const int funcCount = sizeof(funcTable)/sizeof(funcTable[0]);
bool ifMemAlloc[MAX_ARG_N];
int currArgN;
void initializeMemAllocFlag(bool* ifMemAlloc){
    for (int i=0; i<MAX_ARG_N; i++) ifMemAlloc[i] = true;
    currArgN = 0;
}

void handleDtUint8(void** out, uint8_t** in){
	*out = malloc(sizeof(uint8_t));
	*(uint8_t*)(*out) = **in;
	(*in)++;
}
void handleDtUint16(void** out, uint8_t** in){
	uint16_t tmp;
	memcpy(&tmp, *in, 2);
	tmp = ntohs(tmp);
	*out = malloc(sizeof(uint16_t));
	*(uint16_t*)(*out) = tmp;
	(*in)+=2;
}
void handleDtInt32(void** out, uint8_t** in){
	uint32_t tmp;
	memcpy(&tmp, *in, 4);
	tmp = ntohl(tmp);
	*out = malloc(sizeof(uint32_t));
	*(uint32_t*)(*out) = tmp;
	(*in)+=4;
}
void handleAddr(void** out, uint8_t** in){
	ifMemAlloc[currArgN] = false;
    uint32_t tmp;
    memcpy(&tmp, *in, 4);
    tmp = ntohl(tmp);
    *out = (void*)(uintptr_t)tmp;
    (*in)+=4;
}
void handleDtUint8Ptr(void** out, uint8_t** in){
	uint8_t mode = **in;
	(*in)++;
	if (mode & 0x80)
    {
   	 uint8_t len = mode & 0x7F;
   	 uint8_t* val = malloc(len*sizeof(uint8_t));
  		 for (int i = 0; i < len; i++){
  			 void* tmp;
  			 handleDtUint8(&tmp, in);
  			 val[i] = *(uint8_t*)tmp;
  			 free(tmp);
  		 }
  		 *out = val;
  		 return;
  	  }
  	  handleAddr(out, in);
}
void handleDtUint16Ptr(void** out, uint8_t** in){
	uint8_t mode = **in;
	(*in)++;
    if (mode & 0x80) //inline array
    {
   	 uint8_t len = mode & 0x7F;
  		 uint16_t* val = malloc(len*sizeof(uint16_t));
  		 for (int i = 0; i < len; i++){
  			 void* tmp;
  			 handleDtUint16(&tmp, in);
  			 val[i] = *(uint16_t*)tmp;
  			 free(tmp);
  		 }
  		 *out = val;
  		 return;
  	  }
  	  handleAddr(out, in);
}
void handleDtInt32Ptr(void** out, uint8_t** in){
	uint8_t mode = **in;
	(*in)++;
	if (mode & 0x80)
    {
   	 uint8_t len = mode & 0x7F;
   	 uint32_t* val = malloc(len*sizeof(uint32_t));
  		 for (int i = 0; i < len; i++){
  			 void* tmp;
  			 handleDtInt32(&tmp, in);
  			 val[i] = *(int32_t*)tmp;
  			 free(tmp);
  		 }
  		 *out = val;
  		 return;
  	  }
  	  handleAddr(out, in);
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
	if (!buf) {
   	 xil_printf("malloc failed\n\r");
   	 pbuf_free(p);
   	 return ERR_MEM;
	}
	pbuf_copy_partial(p, buf, p->tot_len, 0);
	DataType dt;
    uint8_t cmd_id = buf[0];
    uint8_t *ptr = buf + 1;
    uint32_t ret;
	for (int i = 0; i < funcCount; i++){
    	if(funcTable[i].funcID == cmd_id){
        	void *args[MAX_ARG_N];
        	initializeMemAllocFlag(ifMemAlloc);
        	for (int n = 0; n < funcTable[i].num_args; n++, currArgN++){
            	dt = funcTable[i].arg_types[n];
            	switch (dt){
   				 case DT_UINT8:{
   					 handleDtUint8(&args[n], &ptr);
   					 break;
   				 }
   				 case DT_UINT16:{
   					 handleDtUint16(&args[n], &ptr);
   					 break;
   				 }
   				 case DT_UINT32:
   				 case DT_INT32:{
   					 handleDtInt32(&args[n], &ptr);
   					 break;
   				 }
   				 case DT_UINT8_P:{
   					 handleDtUint8Ptr(&args[n], &ptr);
   					 break;
   				 }
   				 case DT_UINT16_P:{
   					 handleDtUint16Ptr(&args[n], &ptr);
   					 break;
   				 }
   				 case DT_UINT32_P:
   				 case DT_INT32_P:
   				 {
   					 handleDtInt32Ptr(&args[n], &ptr);
   					 break;
   				 }
            	}
        	}
        	ret = funcTable[i].funcName(args);

        	for (int n = 0; n < funcTable[i].num_args; n++){
            	if (ifMemAlloc[n]) free(args[n]);
        	}
        	uint8_t status = (uint8_t)ret;
        	err_t werr = tcp_write(tpcb, &status, sizeof(status), TCP_WRITE_FLAG_COPY);
        	if (werr!=ERR_OK) xil_printf("tcp_write failed: %d\n\r", werr);
        	tcp_output(tpcb);
        	break;
    	}
	}

    free(buf);
    pbuf_free(p);
    return ERR_OK;
}

err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	static int connection = 1;
	tcp_recv(newpcb, recv_callback);
	tcp_arg(newpcb, (void*)(UINTPTR)connection);
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
	xil_printf("TCP server started @ port %d\n\r", port);
	return 0;
}

uint32_t afeSpiRawWrite(void** args){
	uint8_t afeInst = *(uint8_t*)args[0];
	uint16_t addr = *(uint16_t*)args[1];
	uint8_t data = *(uint8_t*)args[2];
	printf("afeSpiRawWrite called:\n");
    printf("  afeInst (uint8)   = %02X\n", afeInst);
    printf("  addr (uint16)   = %04X\n", addr);
    printf("  data (uint8)  = %02X\n", data);
    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiRawRead(void** args){
	uint8_t afeInst = *(uint8_t*)args[0];
	uint16_t addr = *(uint16_t*)args[1];
	uint8_t *readVal = (uint8_t*)args[2];
	printf("afeSpiRawRead called:\n");
    printf("  afeInst (uint8)   = %02X\n", afeInst);
    printf("  addr (uint16)   = %04X\n", addr);
	printf("  readVal points to the addr= %p\n", (void*)readVal);
    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiBurstWrite(void** args)
{
	uint8_t afeInst = *(uint8_t*)args[0];
	uint16_t addr = *(uint16_t*)args[1];
	uint8_t *data = (uint8_t*)args[2];
	uint16_t dataArraySize = *(uint16_t*)args[3];
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

uint32_t afeSpiBurstRead(void** args)
{
	uint8_t afeInst = *(uint8_t*)args[0];
	uint16_t addr = *(uint16_t*)args[1];
	uint16_t dataArraySize = *(uint16_t*)args[2];
	uint8_t *data = (uint8_t*)args[3];
    printf("afeSpiBurstRead called:\n");
    printf("  afeInst (uint8)   = %02X\n", afeInst);
    printf("  addr (uint16)   = %04X\n", addr);
    printf("  dataArraySize (uint16)  = %u\n", dataArraySize);
	printf("  Read data stored in the addr= %p\n", (void*)data);
	return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiRawWriteMulti(void** args)
{
	uint8_t afeInstSel = *(uint8_t*)args[0];
	uint16_t addr = *(uint16_t*)args[1];
	uint8_t data = *(uint8_t*)args[2];
	printf("afeSpiRawWriteMulti called:\n");
    printf("  afeInstSel (uint8)   = %02X\n", afeInstSel);
    printf("  addr (uint16)   = %04X\n", addr);
    printf("  data (uint8)  = %02X\n", data);
    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiRawReadMulti(void** args)
{
	uint8_t afeInstSel = *(uint8_t*)args[0];
	uint16_t addr = *(uint16_t*)args[1];
	uint8_t *readVal = (uint8_t*)args[2];
	printf("afeSpiRawReadMulti called:\n");
    printf("  afeInstSel (uint8)   = %02X\n", afeInstSel);
    printf("  addr (uint16)   = %04X\n", addr);
	printf("  readVal points to the addr= %p\n", (void*)readVal);

    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiBurstWriteMulti(void** args)
{
	uint8_t afeInstSel = *(uint8_t*)args[0];
	uint16_t addr = *(uint16_t*)args[1];
	uint8_t *data = (uint8_t*)args[2];
	uint16_t dataArraySize = *(uint16_t*)args[3];
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
