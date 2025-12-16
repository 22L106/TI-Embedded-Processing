#include <netinet/in.h>// sockaddr_in
#include <arpa/inet.h>

#include <netdb.h> 
#include <stdio.h> //printf perror 
#include <stdlib.h>// mem allocation ,process control,conversion
#include <string.h>// strlen memset 
#include <sys/types.h> 
#include <sys/socket.h>// func def related to socket creation and handling 
#include <unistd.h> // close functions 
#include <stdbool.h>
#define MAX_ARG_N 31
#define MAX_RET_N 15
#define PORT 7000
#define SERVER_IP "192.168.1.10"
int connection = 1;
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

void process_client(int new_socket) 
{
    uint8_t buffer[2048];  // similar to buf = malloc(p->tot_len)
    int bytes;

    while ((bytes = read(new_socket, buffer, sizeof(buffer))) > 0) {
        uint8_t* buf = malloc(bytes);
        memcpy(buf, buffer, bytes);
        uint8_t status;
    DataType dt;
    uint8_t cmd_id = buf[0];
    uint8_t *ptr = buf + 1;
    uint32_t ret;
   for (int i = 0; i < funcCount; i++){
    	if(funcTable[i].funcID == cmd_id){
        	void *args[MAX_ARG_N];
        	bool ifMemAlloc[MAX_ARG_N];
        	for (int n = 0; n < funcTable[i].num_args; n++){
            	dt = funcTable[i].arg_types[n];
            	ifMemAlloc[n] = true;
            	switch (dt){
                	case DT_UINT8:{
                    	uint8_t *val = malloc(sizeof(uint8_t));
                    	*val = *ptr++;
                    	args[n] = val;
                    	break;
                	}
 	case DT_UINT16:{
                    	uint16_t tmp;
                    	memcpy(&tmp, ptr, 2);
                    	tmp = ntohs(tmp);
                    	uint16_t *val = malloc(sizeof(uint16_t));
                    	*val = tmp;
                    	args[n] = val;
                    	ptr+=2;
                    	break;
                	}
                	case DT_UINT32:
                	case DT_INT32:{
                    	uint32_t tmp;
                    	memcpy(&tmp, ptr, 4);
                    	tmp = ntohl(tmp);

           	uint32_t *val = malloc(sizeof(uint32_t));
                    	*val = tmp;
                    	args[n] = val;
                    	ptr+=4;
                    	break;
                	}

 	case DT_UINT8_P:
        case DT_UINT16_P:
        case DT_UINT32_P:
        case DT_INT32_P:
                	{
                    	uint8_t mode = *ptr++;
                    	if (mode & 0x80) //inline array
                    	{
                        	uint8_t len = mode & 0x7F;
                        	int elem_size = (dt==DT_UINT8_P)?1 : (dt==DT_UINT16_P)?2:4;
                        	int need = len*elem_size;
                        	if (elem_size == 2) {
                       		 uint16_t *val = malloc(need);
                       		 memcpy(val, ptr, need);
                       		 for (int i = 0; i < len; i++) val[i] = ntohs(val[i]);
                       		 args[n] = val;
                       		 ptr+= need;
                       		 break;
                   		 }
                   		 else if (elem_size == 4) {
                       		 uint32_t *val = malloc(need);
                       		 memcpy(val, ptr, need);
                       		 for (int i = 0; i < len; i++) val[i] = ntohl(val[i]);
                            	args[n] = val;
                            	ptr+= need;
                            	break;
                   		 }
                   		 else {
                   		 	uint8_t *val = malloc(need);
                   		 	memcpy(val, ptr, need);
	args[n] = val;
                   		 	ptr+= need;
                   		 	break;
                   		 }
                    	}
                    	else{
                        	ifMemAlloc[n] = false;
                        	uint32_t tmp;
                        	memcpy(&tmp, ptr, 4);
                        	tmp = ntohl(tmp);
                        	uint32_t *p = malloc(sizeof(uint32_t));
                                *p = tmp;
                        	args[n] = p;
                        	ptr+=4;
                        	break;
                    	}
                	}
            	}
        	}

                // Call function
                int ret = funcTable[i].funcName(args);

                // Free dynamic args
                for (int n = 0; n < funcTable[i].num_args; n++)
                    if (ifMemAlloc[n]) free(args[n]);

                status = (uint8_t)ret;
                write(new_socket, &status, sizeof(status));  // tcp_write+tcp_output
                break;
            }
        }
        free(buf);
    }

    close(new_socket);  // tcp_close equivalent
}
void handle_client(int client_socket)
{
    printf("Client %d connected\n", connection++);
    process_client(client_socket);  // previously recv_callback()
    close(client_socket);           // equivalent to tcp_close()
}


  
int main()
{
int server_fd, new_socket; //file descriptor for server socket , new_socket
ssize_t valread;// stores num of bytes read from the socket 
struct sockaddr_in address;
int opt = 1;// set socket option
socklen_t addrlen = sizeof(address);//The size of the address structure (used in accept()).


if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
}

if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
}
address.sin_family = AF_INET;
address.sin_addr.s_addr =INADDR_ANY;
address.sin_port = htons(PORT);
//The bind() function binds the server socket to the specified address and port.
if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
}
//The listen() function makes the server socket ready to accept incoming connections.
if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
}
printf("TCP Server started @ port %d\n", PORT);
//The accept() function blocks and waits for an incoming connection
if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
}
//reads data sent by the client through the new_socket connection into the buffer

handle_client(new_socket);
close(new_socket);
close(server_fd);

return 0;}
