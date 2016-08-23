#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "set.h"
#include "collect.h"
#include "crc32.h"
#include "fileop.h"

extern uint8_t Cloud_Code[4];                 //云平台厂商代码
extern uint8_t Username[32];                  //用户名
extern uint8_t Password[16];                  //密码
extern uint8_t Term_Code[16];                 //终端编码
extern uint8_t Term_Desc[32];                 //终端描述
extern uint8_t Addr_Desc[64];                 //地点信息
extern uint8_t Lan_MAC_Addr[6];               //LAN MAC地址
extern uint8_t Lan_IP_Addr[4];                //LAN IP地址
extern uint8_t Lan_IP_Mask[4];                //LAN 子网掩码
extern uint8_t Lan_GW_Addr[4];                //LAN 网关地址
extern uint8_t Lan_AutoIP;                    //LAN 自动获取IP 
extern uint8_t WiFi_MAC_Addr[6];              //WiFi MAC地址
extern uint8_t WiFi_IP_Addr[4];               //WiFi IP地址
extern uint8_t WiFi_IP_Mask[4];               //WiFi 子网掩码
extern uint8_t WiFi_GW_Addr[4];               //WiFi 网关地址
extern uint8_t WiFi_AutoIP;                   //WiFi 自动获取IP 
extern uint8_t DNS1[4];                       //首先DNS服务器
extern uint8_t DNS2[4];                       //备用DNS服务器
extern uint8_t Master_Ip[4];                  //主服务器IP地址
extern uint16_t Master_Port;                  //主服务器侦听端口
extern uint8_t Slave_Ip[4];                   //备用服务器IP地址
extern uint16_t Slave_Port;                   //备用服务器侦听端口
extern uint8_t Comm_Type;                     //通信类型
extern uint8_t Hb_Period;                     //心跳周期
extern uint8_t Resend_Num;                    //重发次数
extern uint8_t Timeout;                       //超时时间
extern uint8_t NTP_Addr[32];                   //密码算法编号
extern uint16_t NTP_Port;                  //NTP服务器侦听端口（缺省：123）
extern uint8_t NTP_Sync_Period;                    //同步周期，单位：天
extern uint8_t NTP_Enable_Flag;                    //使能标志
extern uint8_t Algorithm_id;                   //密码算法编号
extern uint8_t Key[16];                        //密钥
extern uint32_t SessionId;                    //会话ID
extern uint32_t TP_Interval;                  //温度采集周期
extern uint32_t HM_Interval;                  //湿度采集周期
extern uint32_t AIR_Interval;                 //空气质量采集周期
extern uint8_t Video_IP[4];                   //视频源地址
extern uint16_t Video_Port;                   //视频端口号

char buffer[buflen];  //定义数据缓冲区

void process_conn_server(int client_fd)
{
	message_h *head;  
	ssize_t size = 0;  	
  
	for(;;)  
	{  
		//等待读  
		printf("wait read\n");
		for(size = 0;size == 0 ;size = read(client_fd,buffer,buflen));  
		head = (message_h *)buffer;
		printf("read end\n");
		switch( head->Serv_Type )
		{
			case SERV_TYPE_PSET:
				switch( head->Serv_Code )
				{
					case SERV_CODE_BASE:
						set_base(client_fd);         //基本信息设置
						break;
					case SERV_CODE_NET:
						set_net(client_fd);          //网络参数设置
						break;
					case SERV_CODE_INTF:
						set_intf(client_fd);         //接口参数设置
						break;
					case SERV_CODE_NTP:
						set_ntp(client_fd);          //时钟参数设置
						break;
					case SERV_CODE_PKEY:
						set_pkey(client_fd);         //终端密码设置
						break;
					case SERV_CODE_SETINTERVAL:
						set_interval(client_fd);     //传感器采集周期设置
						break;
					default:
						break;
				}
				break;

			case SERV_TYPE_PQRY:
				switch( head->Serv_Code )
				{
					case SERV_CODE_BASE:
						query_base(client_fd);       //基本信息查询
						break;
					case SERV_CODE_NET:
						query_net(client_fd);       //网络参数查询
						break;
					case SERV_CODE_INTF:
						query_intf(client_fd);      //接口参数查询
						break;
					case SERV_CODE_NTP:
						query_ntp(client_fd);       //时钟参数查询
						break;
					case SERV_CODE_PKEY:
						query_pkey(client_fd);      //终端密码查询
						break;
					case SERV_CODE_VIDEOSOURCE:
						query_videosource(client_fd); //视频源地址查询
						break;
					case SERV_CODE_QRYBT:
						query_bluet(client_fd);      //蓝牙设备查询
						break;
					default:
						break;
				}
				break;

			case SERV_TYPE_CTRL:
				switch( head->Serv_Code )
				{
					case SERV_CODE_RESET:
						ctrl_reset(client_fd);       //系统复位
						break;
					case SERV_CODE_DATACLR:
						ctrl_dataclr(client_fd);       //数据清理
						break;
					case SERV_CODE_RESTORE:
						ctrl_restore(client_fd);      //恢复出厂
						break;
					case SERV_CODE_AJUSTTIME:
						ctrl_ajusttime(client_fd);       //时间校准
						break;
					case SERV_CODE_BTCON:
						ctrl_btcon(client_fd);          //连接指定蓝牙设备
						break;
					case SERV_CODE_BTDEL:
						ctrl_btdel(client_fd);          //删除指定蓝牙设备
						break;
					case SERV_CODE_BTCMD:
						ctrl_btcmd(client_fd);          //向指定蓝牙设备发送命令
						break;
					case SERV_CODE_FILERENAME:
						ctrl_filerename(client_fd);      //重命名文件
						break;
					case SERV_CODE_FILEOP:
						ctrl_fileop(client_fd);          //操作文件
						break;
					case SERV_CODE_QRYFILELIST:
						ctrl_qryfilelist(client_fd);       //查询文件列表
						break;
					default:
						break;
				}
				break;
			
			case SERV_TYPE_DQRY:
				switch( head->Serv_Code )
				{
					case SERV_CODE_VER:
						query_ver(client_fd);       //版本及状态信息
						break;
					case SERV_CODE_TIME:
						query_time(client_fd);       //当前日期及时间查询
						break;
					default:
						break;
				}
				break;
			
			default:
				break;
		}

		
	}  
} 

//基本信息设置
void set_base(int client_fd)
{
	printf("set_base\n");
	write_log("set_base\n");

	message_base *message = (message_base *)buffer;
	memcpy( Term_Code, message->Term_Code, sizeof(Term_Code));    //更改终端编码
	memcpy( Term_Desc, message->Term_Desc, sizeof(Term_Desc));    //更改终端描述
	memcpy( Addr_Desc, message->Addr_Desc, sizeof(Addr_Desc));    //更改地点信息

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_PSET;
	message_resp.Resp_Serv_Code = SERV_CODE_BASE;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = message->SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

//网络参数设置 
void set_net(int client_fd)
{
	printf("set_net\n");
	write_log("set_net\n");

	message_net *message = (message_net *)buffer;
	memcpy( Lan_MAC_Addr, message->Lan_MAC_Addr, sizeof(Lan_MAC_Addr));    //更改LAN MAC
	memcpy( Lan_IP_Addr, message->Lan_IP_Addr, sizeof(Lan_IP_Addr));    //更改LAN IP ADDR
	memcpy( Lan_IP_Mask, message->Lan_IP_Mask, sizeof(Lan_IP_Mask));    //更改LAN IP MASK
	memcpy( Lan_GW_Addr, message->Lan_GW_Addr, sizeof(Lan_GW_Addr));    //更改LAN GW ADDR
	Lan_AutoIP = message->Lan_AutoIP;                                   //更改LAN AutoIP
	memcpy( WiFi_MAC_Addr, message->WiFi_MAC_Addr, sizeof(WiFi_MAC_Addr));    //更改WiFi MAC
	memcpy( WiFi_IP_Addr, message->WiFi_IP_Addr, sizeof(WiFi_IP_Addr));    //更改WiFi IP ADDR
	memcpy( WiFi_IP_Mask, message->WiFi_IP_Mask, sizeof(WiFi_IP_Mask));    //更改WiFi IP MASK
	memcpy( WiFi_GW_Addr, message->WiFi_GW_Addr, sizeof(WiFi_GW_Addr));    //更改WiFi GW ADDR
	WiFi_AutoIP = message->WiFi_AutoIP;                                   //更改WiFi AutoIP
	memcpy( DNS1, message->DNS1, sizeof(DNS1));    //更改DNS1
	memcpy( DNS2, message->DNS2, sizeof(DNS2));    //更改DNS2

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_PSET;
	message_resp.Resp_Serv_Code = SERV_CODE_NET;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = message->SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

//接口参数设置 
void set_intf(int client_fd)
{
	printf("set_intf\n");
	write_log("set_intf\n");

	message_intf *message = (message_intf *)buffer;
	memcpy( Master_Ip, message->Master_Ip, sizeof(Master_Ip));    //更改Master_Ip
	memcpy( Slave_Ip, message->Slave_Ip, sizeof(Slave_Ip));    //更改Slave_Ip
	memcpy( Username, message->Username, sizeof(Username));    //更改Username
	memcpy( Password, message->Password, sizeof(Password));    //更改Password
	Master_Port = message->Master_Port;                        //更改Master_Port
	Slave_Port = message->Slave_Port;                        //更改Slave_Port
	Comm_Type = message->Comm_Type;                          //更改Comm_Type
	Hb_Period = message->Hb_Period;                          //更改Hb_Period
	Resend_Num = message->Resend_Num;                        //更改Resend_Num
	Timeout = message->Timeout;                              //更改Timeout

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_PSET;
	message_resp.Resp_Serv_Code = SERV_CODE_INTF;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = message->SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

//时钟参数设置 
void set_ntp(int client_fd)
{
	printf("set_ntp\n");
	write_log("set_ntp\n");

	message_ntp *message = (message_ntp *)buffer;
	memcpy( NTP_Addr, message->NTP_Addr, sizeof(NTP_Addr));    //更改NTP_Addr
	NTP_Port = message->NTP_Port;                              //更改NTP_Port
	NTP_Sync_Period = message->Sync_Period;                    //更改Sync_Period
	NTP_Enable_Flag = message->Enable_Flag;                    //更改Enable_Flag

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_PSET;
	message_resp.Resp_Serv_Code = SERV_CODE_NTP;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = message->SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

//终端密码设置 
void set_pkey(int client_fd)
{
	printf("set_pkey\n");
	write_log("set_pkey\n");

	message_pkey *message = (message_pkey *)buffer;
	Algorithm_id = message->Algorithm_id;
	memcpy( Key, message->Key, sizeof(Key));    //更改终端密码

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_PSET;
	message_resp.Resp_Serv_Code = SERV_CODE_PKEY;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = message->SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

//传感器采集周期设置 
void set_interval(int client_fd)
{
	printf("set_interval\n");
	write_log("set_interval\n");

	message_interval *message = (message_interval *)buffer;
	TP_Interval = message->TP_Interval;
	HM_Interval = message->HM_Interval;
	AIR_Interval = message->AIR_Interval;

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_PSET;
	message_resp.Resp_Serv_Code = SERV_CODE_SETINTERVAL;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = message->SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

//基本信息查询
void query_base(int client_fd)
{
	printf("query_base\n");
	write_log("query_base\n");

	message_base *message_recv = (message_base *)buffer;
	message_base message;
	memset( &message, 0, sizeof(message));

	memcpy( message.Term_Code, Term_Code, sizeof(Term_Code));    //终端编码
	memcpy( message.Term_Desc, Term_Desc, sizeof(Term_Desc));    //终端描述
	memcpy( message.Addr_Desc, Addr_Desc, sizeof(Addr_Desc));    //地点信息

	message.SessionId = message_recv->SessionId;
	strcpy(message.message_head.Term_Code,Term_Code);
	message.message_head.Frame_Head = 0xAAFF;
	message.message_head.Major_Ver = 0;
	message.message_head.Minor_Ver = 0;
	message.message_head.Comm_Type = 0;
	message.message_head.Serv_Type = SERV_TYPE_PQRY;
	message.message_head.Serv_Code = SERV_CODE_BASE;
	message.message_head.Flags = 0x20;
	
	message.message_head.Total_Len = sizeof(message) - 26;
	message.message_head.CRC32 = crc32((uint8_t *)&message.message_head.Seq_Id, sizeof(message)-8-26);
	write(client_fd, &message, sizeof(message)-26); //发送消息
	
	return;
}

//网络参数查询 
void query_net(int client_fd)
{
	printf("query_net\n");
	write_log("query_net\n");

	message_net *message_recv = (message_net *)buffer;
	message_net message;
	memset( &message, 0, sizeof(message));

	memcpy( message.Lan_MAC_Addr, Lan_MAC_Addr, sizeof(Lan_MAC_Addr));    //LAN MAC
	memcpy( message.Lan_IP_Addr, Lan_IP_Addr, sizeof(Lan_IP_Addr));    //LAN IP ADDR
	memcpy( message.Lan_IP_Mask, Lan_IP_Mask, sizeof(Lan_IP_Mask));    //LAN IP MASK
	memcpy( message.Lan_GW_Addr, Lan_GW_Addr, sizeof(Lan_GW_Addr));    //LAN GW ADDR
	message.Lan_AutoIP = Lan_AutoIP;                                   //LAN AutoIP
	memcpy( message.WiFi_MAC_Addr, WiFi_MAC_Addr, sizeof(WiFi_MAC_Addr));    //WiFi MAC
	memcpy( message.WiFi_IP_Addr, WiFi_IP_Addr, sizeof(WiFi_IP_Addr));    //WiFi IP ADDR
	memcpy( message.WiFi_IP_Mask, WiFi_IP_Mask, sizeof(WiFi_IP_Mask));    //WiFi IP MASK
	memcpy( message.WiFi_GW_Addr, WiFi_GW_Addr, sizeof(WiFi_GW_Addr));    //WiFi GW ADDR
	message.WiFi_AutoIP = WiFi_AutoIP;                                   //WiFi AutoIP
	memcpy( message.DNS1, DNS1, sizeof(DNS1));    //DNS1
	memcpy( message.DNS2, DNS2, sizeof(DNS2));    //DNS2

	message.SessionId = message_recv->SessionId;
	strcpy(message.message_head.Term_Code,Term_Code);
	message.message_head.Frame_Head = 0xAAFF;
	message.message_head.Major_Ver = 0;
	message.message_head.Minor_Ver = 0;
	message.message_head.Comm_Type = 0;
	message.message_head.Serv_Type = SERV_TYPE_PQRY;
	message.message_head.Serv_Code = SERV_CODE_NET;
	message.message_head.Flags = 0x20;
	
	message.message_head.Total_Len = sizeof(message) - 26;
	message.message_head.CRC32 = crc32((uint8_t *)&message.message_head.Seq_Id, sizeof(message)-8-26);
	write(client_fd, &message, sizeof(message)-26); //发送消息
	
	return;
}

//接口参数查询 
void query_intf(int client_fd)
{
	printf("query_intf\n");
	write_log("query_intf\n");

	message_intf *message_recv = (message_intf *)buffer;
	message_intf message;
	memset( &message, 0, sizeof(message));

	memcpy( message.Master_Ip, Master_Ip, sizeof(Master_Ip));    //Master_Ip
	memcpy( message.Slave_Ip, Slave_Ip, sizeof(Slave_Ip));    //Slave_Ip
	memcpy( message.Username, Username, sizeof(Username));    //Username
	memcpy( message.Password, Password, sizeof(Password));    //Password
	message.Master_Port = Master_Port;                        //Master_Port
	message.Slave_Port = Slave_Port;                        //Slave_Port
	message.Comm_Type = Comm_Type;                          //Comm_Type
	message.Hb_Period = Hb_Period;                          //Hb_Period
	message.Resend_Num = Resend_Num;                        //Resend_Num
	message.Timeout = Timeout;                              //Timeout

	message.SessionId = message_recv->SessionId;
	strcpy(message.message_head.Term_Code,Term_Code);
	message.message_head.Frame_Head = 0xAAFF;
	message.message_head.Major_Ver = 0;
	message.message_head.Minor_Ver = 0;
	message.message_head.Comm_Type = 0;
	message.message_head.Serv_Type = SERV_TYPE_PQRY;
	message.message_head.Serv_Code = SERV_CODE_INTF;
	message.message_head.Flags = 0x20;
	
	message.message_head.Total_Len = sizeof(message) - 26;
	message.message_head.CRC32 = crc32((uint8_t *)&message.message_head.Seq_Id, sizeof(message)-8-26);
	write(client_fd, &message, sizeof(message)-26); //发送消息
	
	return;
}

//时钟参数查询 
void query_ntp(int client_fd)
{
	printf("query_ntp\n");
	write_log("query_ntp\n");

	message_ntp *message_recv = (message_ntp *)buffer;
	message_ntp message;
	memset( &message, 0, sizeof(message));

	memcpy( message.NTP_Addr, NTP_Addr, sizeof(NTP_Addr));    //NTP_Addr
	message.NTP_Port = NTP_Port;                              //NTP_Port
	message.Sync_Period = NTP_Sync_Period;                    //Sync_Period
	message.Enable_Flag = NTP_Enable_Flag;                    //Enable_Flag

	message.SessionId = message_recv->SessionId;
	strcpy(message.message_head.Term_Code,Term_Code);
	message.message_head.Frame_Head = 0xAAFF;
	message.message_head.Major_Ver = 0;
	message.message_head.Minor_Ver = 0;
	message.message_head.Comm_Type = 0;
	message.message_head.Serv_Type = SERV_TYPE_PQRY;
	message.message_head.Serv_Code = SERV_CODE_NTP;
	message.message_head.Flags = 0x20;
	
	message.message_head.Total_Len = sizeof(message) - 26;
	message.message_head.CRC32 = crc32((uint8_t *)&message.message_head.Seq_Id, sizeof(message)-8-26);
	write(client_fd, &message, sizeof(message)-26); //发送消息
	
	return;
}

void query_pkey(int client_fd)
{
	printf("query_pkey\n");
	write_log("query_pkey\n");

	message_pkey *message_recv = (message_pkey *)buffer;

	message_pkey message;
	memset( &message, 0, sizeof(message));

	message.Algorithm_id = Algorithm_id;
	memcpy( message.Key, Key, sizeof(Key));    //终端密码

	message.SessionId = message_recv->SessionId;
	strcpy(message.message_head.Term_Code,Term_Code);
	message.message_head.Frame_Head = 0xAAFF;
	message.message_head.Major_Ver = 0;
	message.message_head.Minor_Ver = 0;
	message.message_head.Comm_Type = 0;
	message.message_head.Serv_Type = SERV_TYPE_PQRY;
	message.message_head.Serv_Code = SERV_CODE_PKEY;
	message.message_head.Flags = 0x20;
	
	message.message_head.Total_Len = sizeof(message) - 26;
	message.message_head.CRC32 = crc32((uint8_t *)&message.message_head.Seq_Id, sizeof(message)-8-26);
	write(client_fd, &message, sizeof(message)-26); //发送消息
	
	return;
}

void query_videosource(int client_fd)
{
	printf("query_videosource\n");
	write_log("query_videosource\n");

	message_videosource *message_recv = (message_videosource *)buffer;
	message_videosource message;
	memset( &message, 0, sizeof(message));

	message.Source_Port = Video_Port;
	memcpy( message.Source_IP, Video_IP, sizeof(Video_IP));    //终端密码

	message.SessionId = message_recv->SessionId;
	strcpy(message.message_head.Term_Code,Term_Code);
	message.message_head.Frame_Head = 0xAAFF;
	message.message_head.Major_Ver = 0;
	message.message_head.Minor_Ver = 0;
	message.message_head.Comm_Type = 0;
	message.message_head.Serv_Type = SERV_TYPE_PQRY;
	message.message_head.Serv_Code = SERV_CODE_VIDEOSOURCE;
	message.message_head.Flags = 0x20;
	
	message.message_head.Total_Len = sizeof(message) - 26;
	message.message_head.CRC32 = crc32((uint8_t *)&message.message_head.Seq_Id, sizeof(message)-8-26);
	write(client_fd, &message, sizeof(message)-26); //发送消息

	return;
}
 
void query_bluet(int client_fd)
{
	printf("query_bluet\n");
	write_log("query_bluet\n");

	return;
}
  
void ctrl_reset(int client_fd)
{
	printf("ctrl_reset\n");
	write_log("ctrl_reset\n");

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_CTRL;
	message_resp.Resp_Serv_Code = SERV_CODE_RESET;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	strcpy( message_resp.Password, Password );
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}
 
void ctrl_dataclr(int client_fd)
{
	printf("ctrl_dataclr\n");
	write_log("ctrl_dataclr\n");

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_CTRL;
	message_resp.Resp_Serv_Code = SERV_CODE_DATACLR;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	strcpy( message_resp.Password, Password );
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}
 
void ctrl_restore(int client_fd)
{
	printf("ctrl_restore\n");
	write_log("ctrl_restore\n");

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_CTRL;
	message_resp.Resp_Serv_Code = SERV_CODE_RESTORE;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	strcpy( message_resp.Password, Password );
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}
 
void ctrl_ajusttime(int client_fd)
{
	printf("ctrl_ajusttime\n");
	write_log("ctrl_ajusttime\n");

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_CTRL;
	message_resp.Resp_Serv_Code = SERV_CODE_AJUSTTIME;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	strcpy( message_resp.Password, Password );
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

void ctrl_btcon(int client_fd)
{
	printf("ctrl_btcon\n");
	write_log("ctrl_btcon\n");

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_CTRL;
	message_resp.Resp_Serv_Code = SERV_CODE_BTCON;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	strcpy( message_resp.Password, Password );
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

void ctrl_btdel(int client_fd)
{
	printf("ctrl_btdel\n");
	write_log("ctrl_btdel\n");


	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_CTRL;
	message_resp.Resp_Serv_Code = SERV_CODE_BTDEL;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	strcpy( message_resp.Password, Password );
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

void ctrl_btcmd(int client_fd)
{
	printf("ctrl_btcmd\n");
	write_log("ctrl_btcmd\n");

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_CTRL;
	message_resp.Resp_Serv_Code = SERV_CODE_BTCMD;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	strcpy( message_resp.Password, Password );
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}
 
void ctrl_filerename(int client_fd)
{
	printf("ctrl_filerename\n");
	write_log("ctrl_filerename\n");

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_CTRL;
	message_resp.Resp_Serv_Code = SERV_CODE_FILERENAME;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	strcpy( message_resp.Password, Password );
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

void ctrl_fileop(int client_fd)
{
	printf("ctrl_fileop\n");
	write_log("ctrl_fileop\n");

	/*确认报文*/
	message_comm message_resp;
	message_h message_head;

	memset( &message_resp, 0, sizeof(message_resp));
	message_resp.Resp_Serv_Type = SERV_TYPE_CTRL;
	message_resp.Resp_Serv_Code = SERV_CODE_FILEOP;
	message_resp.Error_Code = 0;
	message_resp.SessionId = SessionId;
	strcpy( message_resp.Password, Password );
	memset( &message_head, 0, sizeof(message_h));	  //设置消息头
	strcpy(message_head.Term_Code,Term_Code);
	message_head.Frame_Head = 0xAAFF;
	message_head.Major_Ver = 0;
	message_head.Minor_Ver = 0;
	message_head.Comm_Type = 0;
	message_head.Serv_Type = SERV_TYPE_RESP;
	message_head.Serv_Code = SERV_CODE_RESP;
	message_head.Flags = 0x20;
	

	memcpy(&message_resp.message_head, &message_head, sizeof(message_h));
	message_resp.message_head.Total_Len = sizeof(message_resp) - 26;
	message_resp.SessionId = SessionId;
	message_resp.message_head.CRC32 = crc32((uint8_t *)&message_resp.message_head.Seq_Id, sizeof(message_resp)-8-26);
	write(client_fd, &message_resp, sizeof(message_resp)-26); //发送消息
	
	return;
}

void ctrl_qryfilelist(int client_fd)
{
	printf("ctrl_qryfilelist\n");
	write_log("ctrl_qryfilelist\n");

	return;
}
 
void query_ver(int client_fd)
{
	printf("query_ver\n");
	write_log("query_ver\n");

	return;
}
  
void query_time(int client_fd)
{
	printf("query_time\n");
	write_log("query_time\n");

	return;
}
 
 					
