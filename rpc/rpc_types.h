#include <stdbool.h>

/* returnType */
typedef struct {
    void   *return_val;
    int    return_size;
} return_type;

/* arg list */
typedef struct arg {
    void *arg_val;
    int  arg_size;
    struct arg *next;
} arg_type;

/* Type for the function pointer with which server code registers with the
 * server stub. The first const int argument is the # of arguments, i.e.,
 * array entries in the second argument. */
/**
 * 函数指针
 * @const int: 参数个数
 * @arg_type: 参数数组
 *
 */
typedef return_type (*fp_type)(const int, arg_type *);

/******************************************************************/
/* extern declarations -- you need to implement these 4 functions */
/******************************************************************/

/* The following need to be implemented in the server stub */

/* register_procedure() -- invoked by the app programmer's server code
 * to register a procedure with this server_stub. Note that more than
 * one procedure can be registered */
/*		外部函数声明		*/

/**
 * rpc服务端实现该函数：
 * rpc服务器端注册"程序"
 * 注册一个程序到服务端。
 *
 */
extern bool register_procedure(const char *procedure_name,
	                       const int nparams,
			       fp_type fnpointer);


/* launch_server() -- used by the app programmer's server code to indicate that
 * it wants start receiving rpc invocations for functions that it registered
 * with the server stub.
 *
 * IMPORTANT: the first thing that should happen when launch_server() is invoked
 * is that it should print out, to stdout, the IP address/domain name and
 * port number on which it listens.
 *
 * launch_server() simply runs forever. That is, it waits for a client request.
 * When it receives a request, it services it by invoking the appropriate 
 * application procedure. It returns the result to the client, and goes back
 * to listening for more requests.
 */

/**
 * rpc服务端启动
 * 服务端启动，来接收被注册后程序的rpc远程调用.
 * (1)服务端启动第一件事需要打印到标准输出信息：IP/主机名、服务端口号。
 * (2)rpc服务端服务：接收调用请求；执行调用请求的程序；返回继续监听请求
 *
 *
 */
void launch_server();

/* The following needs to be implemented in the client stub. This is a
 * procedure with a variable number of arguments that the app programmer's
 * client code uses to invoke. The arguments should be self-explanatory.
 *
 * For each of the nparams parameters, we have two arguments: size of the
 * argument, and a (void *) to the argument. */

/**
 * rpc客户端实现该函数：
 * 
 *
 *
 */
extern return_type make_remote_call(const char *servernameorip,  // rpc server address
	                            const int serverportnumber,	 // rpc server port
	                            const char *procedure_name,  // 程序名称 
	                            const int nparams,		 // 参数个数
				    ...);			 // 参数: 自解释，参数的大小和指针
