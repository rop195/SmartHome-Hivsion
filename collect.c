#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "collect.h"
#include "set.h"
#include "crc32.h"


#define BACKLOG 5                //定义侦听队列长度 

char domain[] = "www.gosafenet.net";

pthread_mutex_t sock_mut;        //socket信号量
pthread_t thread[5];
struct sockaddr_in socket_server_addr,socket_send_addr,socket_client_addr;
int server_fd, send_fd, client_fd;
int ConnectStat = TCP_DISCONNECT;

uint8_t Username[32];                  //用户名
uint8_t Password[16];                  //密码
uint8_t Term_Code[16];                 //终端编码
uint8_t Term_Desc[32];                 //终端描述
uint8_t Addr_Desc[64];                 //地点信息
uint8_t Lan_MAC_Addr[6];               //LAN MAC地址
uint8_t Lan_IP_Addr[4];                //LAN IP地址
uint8_t Lan_IP_Mask[4];                //LAN 子网掩码
uint8_t Lan_GW_Addr[4];                //LAN 网关地址
uint8_t Lan_AutoIP;                    //LAN 自动获取IP 
uint8_t WiFi_MAC_Addr[6];              //WiFi MAC地址
uint8_t WiFi_IP_Addr[4];               //WiFi IP地址
uint8_t WiFi_IP_Mask[4];               //WiFi 子网掩码
uint8_t WiFi_GW_Addr[4];               //WiFi 网关地址
uint8_t WiFi_AutoIP;                   //WiFi 自动获取IP 
uint8_t DNS1[4];                       //首先DNS服务器
uint8_t DNS2[4];                       //备用DNS服务器
uint8_t Master_Ip[4];                  //主服务器IP地址
uint16_t Master_Port;                  //主服务器侦听端口
uint8_t Slave_Ip[4];                   //备用服务器IP地址
uint16_t Slave_Port;                   //备用服务器侦听端口
uint8_t Comm_Type;                     //通信类型
uint8_t Hb_Period;                     //心跳周期
uint8_t Resend_Num;                    //重发次数
uint8_t Timeout;                       //超时时间
uint8_t NTP_Addr[32];                   //密码算法编号
uint16_t NTP_Port;                  //NTP服务器侦听端口（缺省：123）
uint8_t NTP_Sync_Period;               //同步周期，单位：天
uint8_t NTP_Enable_Flag;               //使能标志
uint8_t Algorithm_id;                   //密码算法编号
uint8_t Key[16];                        //密钥
uint8_t Cloud_Code[4];                 //云平台厂商代码
uint32_t SessionId;                    //会话ID

void handle_pipe(int sig);
void sig_handler( int sig);

int16_t collect_temp()            //采集温度 
{
	return 30;
}

int16_t collect_humi()            //采集湿度
{
	return 50;
}

int16_t collect_airqua()            //空气质量
{
	return 50;
}

void *sensor_thread()            //本地传感器采集线程
{	
        printf ("sensor_thread start;\n");

	message_sensor sensor;
	message_h message_head;
	int err;

	memset( &sensor, 0, sizeof(message_sensor));

	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(sensor.message_head.Term_Code,Term_Code);
	sensor.message_head.Frame_Head = 0xAAFF;
	sensor.message_head.Comm_Type = 0;
	sensor.message_head.Major_Ver = 0;
	sensor.message_head.Minor_Ver = 0;
	sensor.message_head.Serv_Code = SERV_CODE_SENS;
	sensor.message_head.Serv_Type = SERV_TYPE_DQRY;
	sensor.message_head.Flags = 0x20;
	strcpy( sensor.Equip_Id, "12345678");

       	while(1)
        {
                sensor.Temp = collect_temp();
		sensor.Humi = collect_humi();
		sensor.Airqua = collect_airqua();

		sensor.message_head.Total_Len = sizeof(sensor) -26 ;
		sensor.SessionId = SessionId;
//		strcpy( sensor.Password, Password );
		sensor.message_head.CRC32 = crc32((uint8_t *)&sensor.message_head.Seq_Id, sizeof(sensor)-8-26);
	
                pthread_mutex_lock(&sock_mut);
		if( ConnectStat == TCP_LOGIN )
		{
			int size;  
			size = write(send_fd, &sensor, sizeof(sensor)-26); //发送消息
			if(size<0)
			{
				printf("sensor_thread send error!\n");
				ConnectStat == TCP_DISCONNECT;
			}
			else
				printf("sensor_thread send %d!\n",size);
		}
                pthread_mutex_unlock(&sock_mut);

                sleep(40);
        }

        pthread_exit(NULL);
}

void *connect_thread()         //链路维护线程
{
        printf("connect_thread start;\n");
	message_h message_head;
	int err;
	char revbuf[100];

	memset( &message_head, 0, sizeof(message_h));	  //设置心跳消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Total_Len = 38;
	message_head.Comm_Type = 0;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;	
	message_head.Serv_Code = SERV_CODE_HEATBIT;
	message_head.Serv_Type = SERV_TYPE_LINK;
	message_head.Flags = 0x00;
	message_head.CRC32 = crc32((uint8_t *)&message_head.Seq_Id, sizeof(message_h)-8);

	while(1)  
    	{
		pthread_mutex_lock(&sock_mut);

		if( ConnectStat == TCP_LOGIN )
		{
			int size;  
			size = write(send_fd, &message_head, sizeof(message_head)); //发送心跳消息
			if(size<0)
				printf("connect_thread send error!\n");
			else
				printf("connect_thread send %d!\n",size);
			for(size = 0;size == 0 ;size = read(send_fd,revbuf,100));
			message_comm *message_resp = (message_comm *)revbuf;
			if( message_resp->Resp_Serv_Type == SERV_TYPE_LINK && 
			    message_resp->Resp_Serv_Code == SERV_CODE_HEATBIT && 
			    message_resp->Error_Code != 0 )
			{
				write(send_fd, &message_head, sizeof(message_head)); //第二次发送心跳消息
				for(size = 0;size == 0 ;size = read(send_fd,revbuf,100));
				message_comm *message_resp = (message_comm *)revbuf;
				if( message_resp->Resp_Serv_Type == SERV_TYPE_LINK && 
				    message_resp->Resp_Serv_Code == SERV_CODE_HEATBIT && 
				    message_resp->Error_Code != 0 )
				{
					ConnectStat == TCP_CONNECT;
				}
				
			}
		}

		if( ConnectStat == TCP_CONNECT )
		{
			while( process_login() )
			{
//				sleep(2);
			}
		}

		if( ConnectStat == TCP_DISCONNECT )
		{
			printf("re-connect\n");
			close(send_fd);
			send_fd = socket(AF_INET,SOCK_STREAM, 0);
			while( tcp_connect() )
			{
//				sleep(2);
			}
	
			while( process_login() )
			{
//				sleep(2);
			}
		}

		pthread_mutex_unlock(&sock_mut);
		sleep(30);
	}

        pthread_exit(NULL);
}

void *zigbee_thread()         //ZIGBEE线程
{
        printf("zigbee_thread start;\n");

        pthread_exit(NULL);
}

void *bluetooth_thread()         //Bluetooth线程
{
        printf("buletooth_thread start;\n");

        pthread_exit(NULL);
}

void *mpp_thread()         //视频采集线程
{
        printf("mpp_thread start;\n");

        pthread_exit(NULL);
}

int tcp_connect()
{
	int err;

	err = connect(send_fd,(struct sockaddr *)&socket_send_addr, 
             			sizeof(socket_send_addr));  
	if ( err == 0 )
	{	
		printf("login : connect to server\n");
		ConnectStat = TCP_CONNECT;
		return 0;
	}
	else
	{	
		printf("login : connect error\n");
		ConnectStat = TCP_DISCONNECT;
		return -1;
	}	
}

int process_login()
{
	message_h message_head;
	login_t message_login;
	int err,size;
	char revbuf[100];

	memset( &message_login, 0, sizeof(login_t));      //设置登录名和密码
	strcpy(message_login.Cloud_Code, Cloud_Code); 
	strcpy(message_login.Username, Username); 
	strcpy(message_login.Password, Password); 

	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Total_Len = 40;
	message_head.Comm_Type = 0;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;	
	message_head.Serv_Code = SERV_CODE_LOGON;
	message_head.Serv_Type = SERV_TYPE_LINK;
	message_head.Flags = 0x20;	

	//设置登录消息
	memcpy(&message_login.message_head, &message_head, sizeof(message_h));
	message_login.message_head.Total_Len = sizeof(message_login) -26;
	message_login.SessionId = SessionId;
	message_login.message_head.CRC32 = crc32((uint8_t *)&message_login.message_head.Seq_Id, sizeof(message_login)-8-26);
	
	if( ConnectStat == TCP_CONNECT )
	{	

		write(send_fd, &message_login, sizeof(message_login)-26); //发送登录消息
		for(size = 0;size == 0 ;size = read(send_fd,revbuf,100));
		message_comm *message_resp = (message_comm *)revbuf;
		if( (message_resp->message_head.Flags & ( 1<<5 )) &&
			message_resp->Resp_Serv_Type == SERV_TYPE_LINK && 
			message_resp->Resp_Serv_Code == SERV_CODE_LOGON && 
			message_resp->Error_Code == 0 )
		{
			SessionId = message_resp->SessionId;
			ConnectStat = TCP_LOGIN;
			return 0;
		}
		else
		{
			printf("login : resp error\n"); 
			return -1;
		}
	}
	else
	{   
		return -1;
	}
}

void process_logout()
{
	message_h message_head;

	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Comm_Type = 0;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Serv_Type = SERV_TYPE_LINK;
	message_head.Serv_Code = SERV_CODE_LOGOFF;
	message_head.Flags = 0x00;	
	message_head.CRC32 = crc32((uint8_t *)&message_head.Seq_Id, sizeof(message_h)-8);

	write(send_fd, &message_head, sizeof(message_head)); //发送推出消息
	ConnectStat = TCP_CONNECT;
}

void thread_create(void)
{
        int temp;
        memset(thread, 0, sizeof(thread));
        /*创建线程*/
        if((temp = pthread_create(&thread[0], NULL, connect_thread, NULL)) != 0)
                printf("线程connect创建失败\n");
        else
                printf("线程connect被创建\n");

        if((temp = pthread_create(&thread[1], NULL, sensor_thread, NULL)) != 0)
                printf("线程sensor创建失败\n");
        else
                printf("线程sensor被创建\n");
	
	if((temp = pthread_create(&thread[2], NULL, mpp_thread, NULL)) != 0)
                printf("线程mpp创建失败\n");
        else
                printf("线程mpp被创建\n");

	if((temp = pthread_create(&thread[3], NULL, zigbee_thread, NULL)) != 0)
                printf("线程zigbee创建失败\n");
        else
                printf("线程zigbee被创建\n");

	if((temp = pthread_create(&thread[4], NULL, bluetooth_thread, NULL)) != 0)
                printf("线程bluetooth创建失败\n");
        else
                printf("线程bluetooth被创建\n");
}

void thread_wait(void)
{
        /*等待线程结束*/
        if(thread[0] !=0)
        {    
        	pthread_join(thread[0],NULL);
                printf("线程1已经结束/n");
        }
        if(thread[1] !=0) 
        {  
               	pthread_join(thread[1],NULL);
                printf("线程2已经结束/n");
        }
	if(thread[2] !=0) 
        {  
               	pthread_join(thread[2],NULL);
                printf("线程3已经结束/n");
        }
	if(thread[3] !=0) 
        {  
               	pthread_join(thread[3],NULL);
                printf("线程4已经结束/n");
        }
	if(thread[4] !=0) 
        {  
               	pthread_join(thread[4],NULL);
                printf("线程5已经结束/n");
        }
}

int main()
{
	struct hostent *host;
	struct sigaction action;
	
	strcpy(Cloud_Code, "3214"); 
	strcpy(Username, "myname"); 
	strcpy(Password, "mypassword");
	strcpy(Term_Code, "321409000000");
	Master_Port = 20000;
	SessionId = 1;

	if((host=gethostbyname(domain)) == NULL) 
	{
         	herror("gethostbyname出错！");
         	exit(1);
     	}
        /*用默认属性初始化互斥锁*/
        pthread_mutex_init(&sock_mut,NULL);

	/* 创建客户端TCP套接口 */
	bzero(&socket_send_addr, sizeof(socket_send_addr));
	socket_send_addr.sin_family = AF_INET;
	socket_send_addr.sin_addr = *((struct in_addr *)host->h_addr);
//	socket_send_addr.sin_addr.s_addr = inet_addr("120.25.152.19");
//	socket_send_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	socket_send_addr.sin_port = htons(Master_Port);

	send_fd = socket(AF_INET,SOCK_STREAM, 0);

	printf("login start\n");
	
	while( tcp_connect() )
	{
		sleep(2);
	}
	
	while( process_login() )
	{
		sleep(2);
	}

	signal(SIGINT, sig_handler);       //响应ctrl+c按键
	
	memset(&action, 0, sizeof(action));
	action.sa_handler = handle_pipe;
	sigemptyset( &action.sa_mask );
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, NULL);   //当TCP异常断开，write函数发送失败产生SIGPIPE信号，跳转到handle_pipe

        thread_create();

	/* 创建服务端TCP套接口 */
	bzero(&socket_server_addr, sizeof(socket_server_addr));
	socket_server_addr.sin_family = AF_INET;
	socket_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socket_server_addr.sin_port = htons(Master_Port);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	bind(server_fd, (struct sockaddr *)&socket_server_addr, sizeof(socket_server_addr));
	listen(server_fd, BACKLOG);   //设置监听的队列大小 

	socklen_t addrlen = sizeof(socket_client_addr);  
	while(1)  
    	{

        	//accept返回客户端套接字描述符  
        	client_fd = accept(server_fd,(struct sockaddr *)&socket_client_addr,&addrlen); 
        	if(client_fd < 0)  //出错  
        	{  
            		continue;   //结束此次循环  
        	}  
        	else  
        	{  
            		printf("server : connected to ...\n");  
        	}  

		process_conn_server(client_fd);           //处理数据
		      
	}

        thread_wait();
	close(client_fd);
	close( send_fd );
	close( server_fd );

        return 0;
}

void handle_pipe(int sig)
{
	printf("handle_pipe\n");	
	ConnectStat = TCP_DISCONNECT;	
	return;
}

void sig_handler( int sig)
{
	if(sig == SIGINT)
	{
		printf("ctrl+c has been keydown\n");
		process_logout();
		close( send_fd );
		close( server_fd );
		exit(0);
	}
}

