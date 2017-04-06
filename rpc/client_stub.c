/**
 * References:
 *
 * http://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
 * http://stackoverflow.com/questions/9778806/serializing-a-class-with-a-pointer-in-c
 * http://stackoverflow.com/questions/504810/how-do-i-find-the-current-machines-full-hostname-in-c-hostname-and-domain-info
 *
 * Coding Style:
 *
 * http://www.cs.swarthmore.edu/~newhall/unixhelp/c_codestyle.html
 */

#include "rpc_types.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

const int client_port = 10069;
const int buffer_size = 512;

/**
 * Serializes int values into a character buffer.
 * 序列化int值到字符串数组中
 * 小端方式:低位放在低字节
 */
unsigned char *int_serialize(unsigned char *buffer, int value) {

    int shift = 0;
    const int shift_eight = 8;
    const int shift_sixteen = 16;
    const int shift_twentyfour = 24;

    buffer[shift] = value >> shift;
    buffer[++shift] = value >> shift_eight;
    buffer[++shift] = value >> shift_sixteen;
    buffer[++shift] = value >> shift_twentyfour;

    return buffer + shift + 1;
}

/**
 * Makes a remote procedure call on the specific server.
 */
return_type make_remote_call(const char *servernameorip,
                                const int serverportnumber,
                                const char *procedure_name,
                                const int nparams, ...) {

    return_type rt;
		rt.return_val = NULL;
		rt.return_size = 0;

    // Create server address
    struct sockaddr_in server_socket_address;
    memset((char *)&server_socket_address, 0, sizeof(server_socket_address));
    // 服务端端口号
    server_socket_address.sin_port = htons(serverportnumber);
    // 
    server_socket_address.sin_family = AF_INET;

    // Lookup the server's IP using the hostname provided
    struct hostent *server_ip_address;
    // 查找服务端IP
    server_ip_address = gethostbyname(servernameorip);
    memcpy((void *)&server_socket_address.sin_addr, server_ip_address->h_addr_list[0],
        server_ip_address->h_length);

    // Create client socket and bind address to it
    // UDP 客户端socket
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in client_socket_address;
    memset((char *)&client_socket_address, 0, sizeof(client_socket_address));
    // 
    client_socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    client_socket_address.sin_family = AF_INET;
    // 客户度端口号绑定
    client_socket_address.sin_port = htons(client_port);
    // 绑定客户端地址
    bind(client_socket, (struct sockaddr *)&client_socket_address, sizeof(client_socket_address));

    /**
     * Pack data into buffer
     *
     * Buffer serialization format:
     * [functionSize | functionName]
     * [argumentSize | argument]
     * [argumentSize | argument]
     */

    unsigned char buffer[buffer_size];
    memset(buffer, 0, sizeof(buffer));

    // Pack function name and size in buffer
    // 函数名和名称大小
    int function_name_size = strlen(procedure_name) + 1;
    //（1） 先写程序名的大小 4B
    unsigned char *serial_result = int_serialize(buffer, function_name_size);
    unsigned char *function_name = (unsigned char*)procedure_name;

    // （2）再写程序名  
    int i = 0;
    while (i < function_name_size) {
        serial_result[i] = function_name[i];
        i++;
    } //while
    
    // 当前偏移量
    serial_result = serial_result + function_name_size;
    // （3）写参数个数
    serial_result = int_serialize(serial_result, nparams);

    // Pack argument size and argument in buffer
    va_list var_args;
    va_start(var_args, nparams);
    i = 0;

    while (i < nparams) {
        // Put argument size in buffer
        // （4）参数的size写入
        int arg_size = va_arg(var_args, int);
        serial_result = int_serialize(serial_result, arg_size);

        // Put argument in buffer
        // （5）参数按照字节写入
        void *arg = va_arg(var_args, void *);
        unsigned char *char_arg = (unsigned char*)arg;

        int j = 0;
        while (j < arg_size) {
            serial_result[j] = char_arg[j];
            j++;
        }//while
	// 当前缓冲区偏移量更新
        serial_result = serial_result + arg_size;
        i++;
    }// while

    va_end(var_args);

    /**
     * Make remote procedure call
     * 发送rpc调用信息到服务端包含的数据：低字节 -> 高字节
     * （1）调用程序的名称大小    	4B
     * （2）调用程序名称		实际名称大小
     * （3）参数个数			4B
     * （4）逐个参数			各个参数的字节总大小
     */
     sendto(client_socket, buffer, buffer_size, 0,
     (struct sockaddr *)&server_socket_address, sizeof(server_socket_address));

    /**
     * Listen for the result of remote procedure call
     * 接收rpc处理结果
     */
     struct sockaddr_in remote_address;
     socklen_t addrlen = sizeof(remote_address);

     unsigned char receive_buffer[buffer_size];
     while (1) {
	 // 接收结果，放入receive_buffer
         int receive_length = recvfrom(client_socket, (void *)receive_buffer, buffer_size					,0, (struct sockaddr *)&remote_address, &addrlen);
	 // 处理结果
	 /**
 	  * 返回值：
 	  * （1）返回大小		4B
 	  * （2）返回数据		实际字节大小
          *
          */
         if (receive_length > 0) {
             // Got a good message! Woot!
             unsigned char *return_value_buffer = receive_buffer + 4;
             // （1）返回值大小
             int return_size = *(int*)receive_buffer;
             unsigned char return_value[return_size];

             // （2）返回值字节
             int k = 0;
             while (k < return_size) {
                 return_value[k] = return_value_buffer[k];
                 k++;
             } // while

             memset((unsigned char *)&rt, 0, sizeof(rt));
             rt.return_val = return_value;
             rt.return_size = return_size;
             return rt;
        } //if
    } // while
}
