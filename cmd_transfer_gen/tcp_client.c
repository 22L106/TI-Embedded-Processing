#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "192.168.1.10"
#define SERVER_PORT 7
#define MAX_ARG_N 31
#define MAX_RET_N 15

uint8_t send_buf[1024];
size_t send_len = 0;
uint8_t recv_buf[1024];
size_t recv_len = 0;

typedef enum {DT_UINT8, DT_UINT16, DT_UINT32, DT_UINT8_P, DT_UINT16_P, DT_UINT32_P, DT_INT, DT_INT_P, DT_CHAR, DT_CHAR_P} DataType;

typedef enum {
	FUNC_ID_AFE_SPI_RAW_WRITE = 0,
	FUNC_ID_AFE_SPI_RAW_READ,
	FUNC_ID_AFE_SPI_BURST_WRITE,
	FUNC_ID_AFE_SPI_BURST_READ,
	FUNC_ID_AFE_SPI_RAW_WRITE_MULTI,
	FUNC_ID_AFE_SPI_RAW_READ_MULTI,
	FUNC_ID_AFE_SPI_BURST_WRITE_MULTI
} FunctionID;

typedef struct {
	const char* funcName;
	FunctionID funcID;
	uint8_t num_args;
	uint8_t num_rets;
	DataType arg_types[MAX_ARG_N];
	DataType ret_types[MAX_RET_N];
} funcSpec;
typedef union {
	int32_t   i32;
	uint8_t   u8;
	uint16_t  u16;
	uint32_t  u32;
	uint32_t  ptr;   // for addresses (portable integer type for pointers)
} ArgValue;
typedef enum RET_TYPE {
    TI_AFE_RET_EXEC_PASS = 0,
    TI_AFE_RET_EXEC_FAIL = 1
} RetType_e;



funcSpec functions[] = {
	{"afeSpiRawWrite", FUNC_ID_AFE_SPI_RAW_WRITE, 3, 1, {DT_UINT8, DT_UINT16, DT_UINT8}, {DT_UINT32}},
	{"afeSpiRawRead", FUNC_ID_AFE_SPI_RAW_READ, 2, 1, {DT_UINT8, DT_UINT16}, {DT_UINT32}},
	{"afeSpiBurstWrite", FUNC_ID_AFE_SPI_BURST_WRITE, 4, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P, DT_UINT16}, {DT_UINT32}},
	{"afeSpiBurstRead", FUNC_ID_AFE_SPI_BURST_READ, 3, 1, {DT_UINT8, DT_UINT16, DT_UINT16}, {DT_UINT32}},
	{"afeSpiRawWriteMulti", FUNC_ID_AFE_SPI_RAW_WRITE_MULTI, 3, 1, {DT_UINT8, DT_UINT16, DT_UINT8}, {DT_UINT32}},
	{"afeSpiRawReadMulti", FUNC_ID_AFE_SPI_RAW_READ_MULTI, 2, 1, {DT_UINT8, DT_UINT16}, {DT_UINT32}},
	{"afeSpiBurstWriteMulti", FUNC_ID_AFE_SPI_BURST_WRITE_MULTI, 4, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P, DT_UINT16}, {DT_UINT32}},
	{"func_check_for_uint8ptr", 7, 1, 1, {DT_UINT8_P}, {DT_UINT32}},
	{"func_check_for_uint16ptr", 8, 1, 1, {DT_UINT16_P}, {DT_UINT32}},
	{"func_check", 9, 8, 1, {DT_INT, DT_UINT8, DT_UINT16, DT_UINT32, DT_UINT8_P, DT_UINT16_P, DT_UINT32_P, DT_INT_P}, {DT_UINT32}},
	{"func_check_for_space", 10, 2, 1, {DT_INT_P, DT_UINT8_P}, {DT_UINT32}}
};

funcSpec* lookupFunc(char* cmd, int func_n){
	for (int i = 0; i<func_n; i++){
    	if (strcmp(functions[i].funcName, cmd) == 0) return &functions[i];
	}
	return NULL;
}

void space_strip(const char *input, char *output){
	const char *start = input;
	while (isspace((unsigned char)*start)){
    	start++;
	}
	int i;
	for (i=strlen(start)-1; isspace(start[i]); i--);
	strncpy(output, start, i+1);
	output[i+1] = '\0';
}

void declaration(){
	char line[512];
	while(1){
    	printf(">>> ");
    	fflush(stdout);
    	if (!fgets(line, sizeof(line), stdin)) break;
    	line[strcspn(line, "\n")] = 0;
    	/*
    	variable initialization logic here
    	*/
    	if (strncmp(line, "enddecl", 7) == 0) return;
	}
}

static int parse_scalar(const char *arg_raw, DataType type, ArgValue *out) {
	char arg[200];
	space_strip(arg_raw, arg);
	char *endptr;
	unsigned long uval;
	long ival;

	switch (type) {
	case DT_INT:
    	ival = strtol(arg, &endptr, 0);
    	if (*endptr != '\0') return -1;
    	out->i32 = (int32_t)ival;
    	printf("int result %d size %zu\n", out->i32, sizeof(out->i32));
    	int32_t i32 = htonl(out->i32);
    	memcpy(send_buf + send_len, &i32, sizeof(i32));
    	send_len += sizeof(i32);
    	break;
    	return 0;
   	 
	case DT_UINT8:
    	uval = strtoul(arg, &endptr, 0);
    	if (*endptr != '\0') return -1;
    	if (uval > UINT8_MAX) {
        	printf("Value %lu exceeds uint8_t max, clamped\n", uval);
        	out->u8 = UINT8_MAX;
    	} else {
        	out->u8 = (uint8_t)uval;
    	}
    	printf("uint8 result %u size %zu\n", out->u8, sizeof(out->u8));
    	memcpy(send_buf + send_len, &(out->u8), sizeof(out->u8));
    	send_len += sizeof(out->u8);
    	return 0;

	case DT_UINT16:
    	uval = strtoul(arg, &endptr, 0);
    	if (*endptr != '\0') return -1;
    	if (uval > UINT16_MAX) {
        	printf("Value %lu exceeds uint16_t max, clamped\n", uval);
        	out->u16 = UINT16_MAX;
    	} else {
        	out->u16 = (uint16_t)uval;
    	}
    	printf("uint16 result %u size %zu\n", out->u16, sizeof(out->u16));
    	uint16_t u16 = htons(out->u16);
    	memcpy(send_buf + send_len, &u16, sizeof(u16));
    	send_len += sizeof(u16);
    	return 0;

	case DT_UINT32:
    	uval = strtoul(arg, &endptr, 0);
    	if (*endptr != '\0') return -1;
    	if (uval > UINT32_MAX) {
        	printf("Value %lu exceeds uint32_t max, clamped\n", uval);
        	out->u32 = UINT32_MAX;
    	} else {
        	out->u32 = (uint32_t)uval;
    	}
    	printf("uint32 result %u size %zu\n", out->u32, sizeof(out->u32));
    	uint32_t u32 = htonl(out->u32);
    	memcpy(send_buf + send_len, &u32, sizeof(u32));
    	send_len += sizeof(u32);
    	return 0;

	case DT_INT_P:	// fallthrough
	case DT_UINT8_P:  // fallthrough
	case DT_UINT16_P: // fallthrough
	case DT_UINT32_P:
    	if (strncmp(arg, "0x", 2) == 0) {
        	uval = strtoul(arg, &endptr, 0);
        	if (*endptr != '\0') return -1;
        	out->ptr = (uint32_t)uval;
        	printf("pointer address 0x%lx size %zu\n",
               	(unsigned long)out->ptr, sizeof(out->ptr));
        	uint8_t sep = 0;
        	memcpy(send_buf + send_len, &sep, sizeof(sep));
        	send_len += sizeof(sep);
        	uint32_t ptr = htonl(out->ptr);
        	memcpy(send_buf + send_len, &ptr, sizeof(ptr));
        	send_len += sizeof(ptr);
        	return 0;
    	}
    	// otherwise array form {...}, handled separately
    	return 1; // signal "array form"

	default:
    	return -1;
	}
}

// helper to parse array form: "{1,2,3}"
static void parse_array(const char *arg_raw, DataType type) {
	char arg[128];
	space_strip(arg_raw, arg);
    
	char buf[128];
	uint8_t n_ele = 0;
	strncpy(buf, arg + 1, sizeof(buf) - 1); // skip '{'
	buf[sizeof(buf)-1] = '\0';
	char *tok_raw = strtok(buf, ",}");

	for (int i=0; arg_raw[i]; i++){
    	if (arg_raw[i] == ',') n_ele++;
	}
    
	uint8_t arrInd_noEle = (((int8_t)1) << 7) | (n_ele+1);
	memcpy(send_buf + send_len, &arrInd_noEle, sizeof(arrInd_noEle));
	send_len += sizeof(arrInd_noEle);
    
	ArgValue ele;
	while (tok_raw) {
    	n_ele += 1;
    	char tok[100];
    	space_strip(tok_raw, tok);
    	char *endptr;
    	unsigned long uval = strtoul(tok, &endptr, 0);
    	if (*endptr != '\0') {
        	printf("invalid array element: %s\n", tok);
    	}
    	else {
        	switch (type) {
        	case DT_UINT8_P:
            	ele.u8 = (uint8_t)(uval > UINT8_MAX ? UINT8_MAX : uval);
            	printf("uint8p result %u\n", ele.u8);
            	memcpy(send_buf + send_len, &(ele.u8), sizeof(ele.u8));
            	send_len += sizeof(ele.u8);
            	break;
        	case DT_UINT16_P:
            	ele.u16 = (uint16_t)(uval > UINT16_MAX ? UINT16_MAX : uval);
            	printf("uint16p result %u\n", ele.u16);
            	uint16_t u16 = htons(ele.u16);
            	memcpy(send_buf + send_len, &u16, sizeof(u16));
            	send_len += sizeof(u16);
            	break;
        	case DT_UINT32_P:
            	ele.u32 = (uint32_t)(uval > UINT32_MAX ? UINT32_MAX : uval);
            	printf("uint32p result %u\n", ele.u32);
            	uint32_t u32 = htonl(ele.u32);
            	memcpy(send_buf + send_len, &u32, sizeof(u32));
            	send_len += sizeof(u32);
            	break;
        	case DT_INT_P:
            	ele.i32 = (int32_t)(uval > INT32_MAX ? INT32_MAX : uval);
            	printf("intp result %u\n", ele.i32);
            	int32_t i32 = htonl(ele.i32);
            	memcpy(send_buf + send_len, &i32, sizeof(i32));
            	send_len += sizeof(i32);
            	break;
        	default: break;
        	}
    	}
    	tok_raw = strtok(NULL, ",}");
	}
}

void arg_handle(char *arg, DataType argType) {
	ArgValue val;
	int ret = parse_scalar(arg, argType, &val);
	if (ret == -1) {
    	printf("invalid argument (type) found: %s\n", arg);
	} else if (ret == 1) {
    	// means pointer type in array form
    	parse_array(arg, argType);
	}
}

void parse_args(const char *args, const funcSpec *cmdSpec) {
	char buf[256];
	char buf_raw[256];
	char arg_buf[256];
	char* arg;
	char* temp;
	char* end;
	int arg_len;
    
	int arg_n = cmdSpec->num_args;
	space_strip(args, buf);
	for (int n = 0; n < arg_n; n++) {
    	arg_len = 0;
    	DataType arg_t = cmdSpec->arg_types[n];
   	 
    	if (buf[0] == '{'){
        	end = strchr(buf, '}');
        	if (end){
            	size_t len = end - buf + 1;
            	strncpy(arg_buf, buf, len);
            	arg_buf[len] = '\0';
            	arg = arg_buf;
        	}
        	arg_handle(arg, arg_t);
        	space_strip(buf+strlen(arg), buf);
        	strcpy(buf_raw, buf+1);
        	space_strip(buf_raw, buf);           	 
    	}
    	else{
        	arg = strtok(buf, ",");
        	arg_handle(arg, arg_t);
        	arg_len = strlen(arg);
        	strcpy(buf_raw, buf+arg_len+1);
        	space_strip(buf_raw, buf);
    	}
	}
}

void print_buf(const uint8_t *buf, size_t len) {
	printf("sendbuf (%zu bytes): ", len);
	for (size_t i = 0; i < len; i++) {
    	printf("%02X ", buf[i]);   // print each byte as 2-digit hex
	}
	printf("\n");
}

int main(){
	int sock;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0))<0){
    	perror("Socket creation failed");
    	return -1;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVER_PORT);
    
	if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0){
    	perror("Invalid address / not supported");
    	return -1;
	}
    
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
    	perror("Connection failed");
    	return -1;
	}
    
	printf("Connected to FPGA echo server at %s:%d\n", SERVER_IP, SERVER_PORT);
	uint32_t num;
	char line[256];
	char cmd_raw[64];
	char cmd[64];
	char args[224];
    
	funcSpec* cmdSpec;
	int func_n = sizeof(functions)/sizeof(funcSpec);
	while(1){
    	printf(">>> ");
    	fflush(stdout); //flushes the buffer
    	if (!fgets(line, sizeof(line), stdin)) break;  
    	line[strcspn(line, "\n")] = 0; // strcspn <string complementary span> returns number of characters in 'line' before first occurrence of '\n'
   	 
    	if (strncmp(line, "quit", 4) == 0) break; //there is trailing \n after quit, so we either strip the \n's or compare first 4 chars
    	if (strncmp(line, "decl", 4) == 0) {declaration(); continue;}
   	 
//afeSpiRawWrite(1, 0x0F2F, 0x25)   

    	//command handling
    	size_t cmd_len = strcspn(line, "(");   // cmd_len = 14
    	strncpy(cmd_raw, line, cmd_len);    	// cmd = 'afeSpiRawWrite'
    	space_strip(cmd_raw, cmd);
    	cmd[cmd_len] = '\0';            	// cmd = 'afeSpiRawWrite\0'
    	if (!(cmdSpec = lookupFunc(cmd, func_n))){
            	printf("command not found\n");
            	continue;
    	}
    	uint8_t cmdId = cmdSpec -> funcID;
    	memcpy(send_buf + send_len, &cmdId, sizeof(cmdId));
    	send_len += sizeof(cmdId);
   	 
   	 
    	strcpy(args, line+cmd_len+1);	// args = "1, 0x0F2F, 0x25)"
    	size_t args_len = strcspn(args, ")"); //args_len = 14
    	strncpy(args, args, args_len); 	// args = "1, 0x0F2F, 0x25"
    	args[args_len] = '\0';        	// args = "1, 0x0F2F, 0x25\0"
    	parse_args(args, cmdSpec);
    	print_buf(send_buf, send_len);
   	 
    	ssize_t sent = send(sock, send_buf, send_len, 0);
    	if(sent < 0){
        	perror("send failed");
        	break;
    	}
   	 
    	memset(send_buf, 0, sizeof(send_buf));
    	send_len = 0;
   	 
    	ssize_t n = recv(sock, recv_buf, sizeof(recv_buf), 0);
    	if (n <= 0){
        	printf("Connection closed by server.\n");
        	close(sock);
        	exit(1);
    	}
    
    printf("received buffer:");
    for (int i = 0; i<n; i++) printf("%02x ", recv_buf[i]);
    printf("\n");

    	uint8_t status = recv_buf[n-1];
   	 
    	//recv_buf -> dataArraySize, Array, status
    	if (cmdSpec -> funcID == FUNC_ID_AFE_SPI_BURST_READ) {
        	uint16_t arraySize;
        	memcpy(&arraySize, recv_buf, sizeof(uint16_t));
        	arraySize = ntohs(arraySize);
        	printf("Burst Read (size: %u bytes): \n", arraySize);
       	 
        	uint8_t* readValues;
        	readValues = malloc(sizeof(arraySize));
        	memcpy(readValues, recv_buf+sizeof(uint16_t), arraySize);
        	for (int i = 0; i < arraySize; i++)
            	printf("%02x ", readValues[i]);
        	printf("\n");
    	}
    	//recv_buf -> readByte, status
    	else if (cmdSpec -> funcID == FUNC_ID_AFE_SPI_RAW_READ || cmdSpec -> funcID == FUNC_ID_AFE_SPI_RAW_READ_MULTI){
        	uint8_t readValue;
        	printf("Read Byte : %02x\n", recv_buf[0]);
    	}
   	 
    	//recv_buf -> status
    	if (status == TI_AFE_RET_EXEC_PASS) printf("EXEC_PASS\n");
    	else if (status == TI_AFE_RET_EXEC_FAIL) printf("EXEC_FAIL\n");
    	else printf("Microblaze returned: UNKNOWN (0x%02X)\n", status);
    	}
   	 
	close(sock);
	return 0;
}
