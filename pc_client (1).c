#include <stdio.h>
#include <sys/time.h>  // for gettimeofday

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>


#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7000
#define MAX_ARG_N 31
#define MAX_RET_N 15

uint8_t send_buf[1024];
size_t send_len = 0;
uint8_t recv_buf[1024];
size_t recv_len = 0;



typedef enum {DT_UINT8, DT_UINT16, DT_UINT32, DT_UINT8_P, DT_UINT16_P, DT_UINT32_P, DT_INT, DT_INT_P, DT_CHAR, DT_CHAR_P} Data_t;

typedef struct{ const char*name;
                uint8_t id;
                uint8_t args;
                uint8_t rets;
                Data_t arg_typ[MAX_ARG_N];
                Data_t ret_typ[MAX_RET_N];
}funcSpec_t;//struct funcspec


typedef union    
{
    int32_t   i32;
    uint8_t   u8;
    uint16_t  u16;
    uint32_t  u32;
    uint32_t  ptr;   // for addresses (portable integer type for pointers)
} ArgValue;


typedef enum RET_TYPE {
	TI_AFE_RET_EXEC_PASS = 0,
	TI_AFE_RET_EXEC_FAIL = 1
} Return_t;

//struct funcspec_t array 
funcSpec_t functions[] = {
    {"afeSpiRawWrite", 0, 3, 1, {DT_UINT8, DT_UINT16, DT_UINT8}, {DT_UINT32}},
    {"afeSpiRawRead", 1, 3, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P}, {DT_UINT32}},
    {"afeSpiBurstWrite", 2, 4, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P, DT_UINT16}, {DT_UINT32}},
    {"afeSpiBurstRead", 3, 4, 1, {DT_UINT8, DT_UINT16, DT_UINT16, DT_UINT8_P}, {DT_UINT32}},
    {"afeSpiRawWriteMulti", 4, 3, 1, {DT_UINT8, DT_UINT16, DT_UINT8}, {DT_UINT32}},
    {"afeSpiRawReadMulti", 5, 3, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P}, {DT_UINT32}},
    {"afeSpiBurstWriteMulti", 6, 4, 1, {DT_UINT8, DT_UINT16, DT_UINT8_P, DT_UINT16}, {DT_UINT32}},
    {"func_check_for_uint8ptr", 7, 1, 1, {DT_UINT8_P}, {DT_UINT32}},
    {"func_check_for_uint16ptr", 8, 1, 1, {DT_UINT16_P}, {DT_UINT32}},
    {"func_check", 9, 8, 1, {DT_INT, DT_UINT8, DT_UINT16, DT_UINT32, DT_UINT8_P, DT_UINT16_P, DT_UINT32_P, DT_INT_P}, {DT_UINT32}},
    {"func_check_for_space", 10, 2, 1, {DT_INT_P, DT_UINT8_P}, {DT_UINT32}}
};

funcSpec_t* lookupFunc(char* cmd, int func_n){
    for (int i = 0; i<func_n; i++){
        if (strcmp(functions[i].name, cmd) == 0) return &functions[i];
    }
    return NULL;
}

void space_strip(const char *input, char *output){
    const char *start = input;
    while (isspace((unsigned char)*start)){
        start++; //skip leading  spaces 
    }
    int i;
    for (i=strlen(start)-1; isspace(start[i]); i--);//scanning for the last non space character
    strncpy(output, start, i+1);//copy only 
    output[i+1] = '\0'; //add null terminator
}

void declaration(){
    char line[512];
    while(1){
        printf(">>> ");
        fflush(stdout);//force the contents to dispalyed
        if (!fgets(line, sizeof(line), stdin)) break;//char *fgets(char *str, int n, FILE *stream);
        line[strcspn(line, "\n")] = 0;//the index of the newline character replaced with null terminator 
        /*
        variable initialization logic here
        */
        if (strncmp(line, "enddecl", 7) == 0) return;
    }
}
static int parse_scalar(const char *arg_raw, Data_t type, ArgValue *out) {
    char arg[200];
    space_strip(arg_raw, arg);// input arg as arg_raw and convert into ArgVal union type
    char *endptr;
    unsigned long uval;
    long ival;

 switch (type) {
    case DT_INT:
        ival = strtol(arg, &endptr, 0);// converts the (arg)string into a long integer base is automatically detected
        if (*endptr != '\0') return -1;
        out->i32 = (int32_t)ival;//typecast to 32bit sig
        printf("int result %d size %zu\n", out->i32, sizeof(out->i32));
        int32_t i32 = htonl(out->i32);//host to network byte order big endian
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

    case DT_INT_P:    // fallthrough
    case DT_UINT8_P:  // fallthrough
    case DT_UINT16_P: // fallthrough
    case DT_UINT32_P:// pointer (address) declaration starts with 0x
  if (strncmp(arg, "0x", 2) == 0) {
            uval = strtoul(arg, &endptr, 0);//string to unsigned long integer
            if (*endptr != '\0') return -1;
            out->ptr = (uint32_t)uval;
            printf("pointer address 0x%lx size %zu\n",
                   (unsigned long)out->ptr, sizeof(out->ptr));
            uint8_t sep = 0;//one byte separator set to 0
            memcpy(send_buf + send_len, &sep, sizeof(sep));//sending a sep byte 0 before ponter(address) start of pointer data.
            send_len += sizeof(sep);
            uint32_t ptr = htonl(out->ptr);
            memcpy(send_buf + send_len, &ptr, sizeof(ptr));
            send_len += sizeof(ptr);
            return 0;
        }
   return 1; // signal "array form"

    default:
        return -1;
    }
}

static void parse_array(const char *arg_raw, Data_t type) {
    char arg[128];
    space_strip(arg_raw, arg);
    
    char buf[128];
    uint8_t n_ele = 0;
    strncpy(buf, arg + 1, sizeof(buf) - 1); // skip '{' arg+1 start from the elem
    buf[sizeof(buf)-1] = '\0';
    char *tok_raw = strtok(buf, ",}");//Splits the buffer by comma (,) or closing brace (})

    for (int i=0; arg_raw[i]; i++){
        if (arg_raw[i] == ',') n_ele++;//Counts the commas to find number of elements.
    }
    
  uint8_t arrInd_noEle = (((int8_t)1) << 7) | (n_ele+1);//sets 7th bit as 1 (MSB) -array indicator byte ,and low 7 bit  stores no of element in the array 
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
                ele.u8 = (uint8_t)(uval > UINT8_MAX ? UINT8_MAX : uval);//assign value within max range or clam it max range 
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
        tok_raw = strtok(NULL, ",}");//Subsequent calls should pass NULL to tell strtok to continue from the last position
    }
}

void arg_handle(char *arg, Data_t argType) {
    ArgValue val;
    int ret = parse_scalar(arg, argType, &val);
    if (ret == -1) {
        printf("invalid argument (type) found: %s\n", arg);
    } else if (ret == 1) {
        // means pointer type in array form
        parse_array(arg, argType);
    }
}

void parse_args(const char *args_i, const funcSpec_t *cmdSpec) {
    char buf[256];
    char buf_raw[256];
    char arg_buf[256];
    char* arg;
    char* temp;
    char* end;
    int arg_len;
    
    int arg_n = cmdSpec->args;
    space_strip(args_i, buf);
    for (int n = 0; n < arg_n; n++) {
        arg_len = 0;
        Data_t arg_t = cmdSpec->arg_typ[n];
        
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
            strcpy(buf_raw, buf+arg_len+1);//moves pointer to next argument
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


int main()

{
struct timeval t_start, t_end;

int status, valread, client_fd;//file descriptor of the client socket
struct sockaddr_in serv_addr;

if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
}
bzero(&serv_addr, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(SERVER_PORT);


if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
}
//inet_pton() function to convert the string representation of the IP address ("127.0.0.1") into binary form and stores it in serv_addr.sin_addr.

if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
    printf("\nConnection Failed \n");
    return -1;
}
printf("Connected to FPGA echo server at %s:%d\n", SERVER_IP, SERVER_PORT);
    uint32_t num;
    char line[256];
    char cmd_raw[64];
    char cmd[64];
    char args_i[224];
    
 funcSpec_t* cmdSpec;
    int func_n = sizeof(functions)/sizeof(funcSpec_t);
    while(1){
        printf(">>> ");
        fflush(stdout); //flushes the buffer
        if (!fgets(line, sizeof(line), stdin)) break;  
        line[strcspn(line, "\n")] = 0; // strcspn <string complementary span> returns number of characters in 'line' before first occurrence of '\n' replace it with 0
        
        if (strncmp(line, "quit", 4) == 0) break; //there is trailing \n after quit, so we either strip the \n's or compare first 4 chars
        if (strncmp(line, "decl", 4) == 0) {declaration(); continue;}
        //afeSpiRawWrite(1, 0x0F2F, 0x25)   

        //command handling
        size_t cmd_len = strcspn(line, "(");   // cmd_len = 14
        strncpy(cmd_raw, line, cmd_len);        // cmd = 'afeSpiRawWrite'
        space_strip(cmd_raw, cmd);
        cmd[cmd_len] = '\0';                // cmd = 'afeSpiRawWrite\0'
        if (!(cmdSpec = lookupFunc(cmd, func_n))){
                printf("command not found\n");
                continue;
        }
        uint8_t cmdId = cmdSpec -> id;
        memcpy(send_buf + send_len, &cmdId, sizeof(cmdId));
        send_len += sizeof(cmdId);
        strcpy(args_i, line+cmd_len+1);    // args = "1, 0x0F2F, 0x25)"
        size_t args_len = strcspn(args_i, ")"); //args_len = 14
        strncpy(args_i, args_i, args_len);     // args = "1, 0x0F2F, 0x25"
        args_i[args_len] = '\0';            // args = "1, 0x0F2F, 0x25\0"
        parse_args(args_i, cmdSpec);
        print_buf(send_buf, send_len);
        
        ssize_t sent = send(client_fd, send_buf, send_len, 0);
        if(sent < 0){
            perror("send failed");
            break;
        }
         gettimeofday(&t_start, NULL);//sent time
        memset(send_buf, 0, sizeof(send_buf));
        send_len = 0;
        
        ssize_t n = recv(client_fd, recv_buf, sizeof(recv_buf), 0);
        if (n <= 0){
            printf("Connection closed by server.\n");
            close(client_fd);
            exit(1);
        }
        
        if (*recv_buf == TI_AFE_RET_EXEC_PASS) printf("EXEC_PASS\n");
        else if (*recv_buf == TI_AFE_RET_EXEC_FAIL) printf("EXEC_FAIL\n");
        else printf("Microblaze returned: UNKWOWN (0x%02X)\n", *recv_buf);
gettimeofday(&t_end, NULL);//recv time
  double latency_ms = (t_end.tv_sec - t_start.tv_sec) * 1000.0
                      + (t_end.tv_usec - t_start.tv_usec) / 1000.0;

    printf("Round-trip latency: %.3f ms\n", latency_ms);

        }
     
    close(client_fd);
    return 0;
}



/*
func(client_fd);
close(client_fd);

return 0;
}


void func(int client_fd)
{
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        write(client_fd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(client_fd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}*/
