#ifndef _COLLECT_H_
#define _COLLECT_H_

#include <stdint.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED -1
#endif

#pragma pack(1)

typedef struct _message_h {           //消息头结构体
uint16_t Frame_Head;
uint16_t Total_Len;
uint32_t CRC32;           
uint32_t Seq_Id;       
uint8_t Comm_Type;           
uint8_t Enc_Type;        
uint8_t Major_Ver;        
uint8_t Minor_Ver;      
uint8_t Term_Code[12];
uint16_t Serv_Type;
uint16_t Serv_Code;
uint8_t Flags;
uint8_t Priority;
uint8_t Reserved[4];   
}message_h;                             

typedef struct _message_comm {          //通用消息结构体
message_h message_head;
uint16_t Resp_Serv_Type;                //响应服务类型
uint16_t Resp_Serv_Code;                //响应服务代码
int Error_Code;                         //错误代码， 0:表示正确
uint32_t SessionId;
uint8_t Password[16];
uint8_t Timestamp[6];
int Expire_time; 
}message_comm;                           

typedef struct _login_t {              //登录消息结构体
message_h message_head;
uint8_t Cloud_Code[4];                 //云平台厂商代码
uint8_t Username[32];                  //用户名
uint8_t Password[16];                  //密码
uint32_t SessionId;
uint8_t passwd[16];
uint8_t Timestamp[6];
int Expire_time; 
}login_t;                               

typedef struct _message_base {          //基本信息结构体
message_h message_head;
uint8_t Term_Code[16];                  //终端编码
uint8_t Term_Desc[32];                  //终端描述
uint8_t Addr_Desc[64];                  //地点信息
uint32_t SessionId;
uint8_t Password[16];
uint8_t Timestamp[6];
int Expire_time; 
}message_base;

typedef struct _message_net {          //网络参数结构体
message_h message_head;
uint8_t Lan_MAC_Addr[6];                  //LAN MAC地址
uint8_t Lan_IP_Addr[4];                   //LAN IP地址
uint8_t Lan_IP_Mask[4];                   //LAN 子网掩码
uint8_t Lan_GW_Addr[4];                   //LAN 网关地址
uint8_t Lan_AutoIP;                       //LAN 自动获取IP 
uint8_t WiFi_MAC_Addr[6];                 //WiFi MAC地址
uint8_t WiFi_IP_Addr[4];                  //WiFi IP地址
uint8_t WiFi_IP_Mask[4];                  //WiFi 子网掩码
uint8_t WiFi_GW_Addr[4];                  //WiFi 网关地址
uint8_t WiFi_AutoIP;                      //WiFi 自动获取IP 
uint8_t DNS1[4];                          //首先DNS服务器
uint8_t DNS2[4];                          //备用DNS服务器
uint32_t SessionId;
uint8_t Password[16];
uint8_t Timestamp[6];
int Expire_time; 
}message_net;

typedef struct _message_intf {          //接口参数结构体
message_h message_head;
uint8_t Master_Ip[4];                  //主服务器IP地址
uint16_t Master_Port;                  //主服务器侦听端口
uint8_t Slave_Ip[4];                   //备用服务器IP地址
uint16_t Slave_Port;                   //备用服务器侦听端口
uint8_t Username[32];                  //用户名
uint8_t Password[16];                  //密码
uint8_t Comm_Type;                     //通信类型
uint8_t Hb_Period;                     //心跳周期
uint8_t Resend_Num;                    //重发次数
uint8_t Timeout;                       //超时时间
uint32_t SessionId;
uint8_t passwd[16];
uint8_t Timestamp[6];
int Expire_time; 
}message_intf;      

typedef struct _message_ntp {          //时钟参数结构体
message_h message_head;
uint8_t NTP_Addr[32];                   //密码算法编号
uint16_t NTP_Port;                  //NTP服务器侦听端口（缺省：123）
uint8_t Sync_Period;                    //同步周期，单位：天
uint8_t Enable_Flag;                    //使能标志
uint32_t SessionId;
uint8_t Password[16];
uint8_t Timestamp[6];
int Expire_time; 
}message_ntp;                           

typedef struct _message_pkey {          //终端密码结构体
message_h message_head;
uint8_t Algorithm_id;                   //密码算法编号
uint8_t Key[16];                        //密钥
uint32_t SessionId;
uint8_t Password[16];
uint8_t Timestamp[6];
int Expire_time; 
}message_pkey;     

typedef struct _message_interval {      //传感器采集周期结构体
message_h message_head;
uint32_t TP_Interval;                   //温度采集周期
uint32_t HM_Interval;                   //湿度采集周期
uint32_t AIR_Interval;                  //空气质量采集周期
uint32_t SessionId;
uint8_t Password[16];
uint8_t Timestamp[6];
int Expire_time; 
}message_interval;     

typedef struct _message_videosource {      //视频源结构体
message_h message_head;
uint8_t Source_IP[4];                   //视频源地址
uint16_t Source_Port;                   //视频源端口号
uint32_t SessionId;
uint8_t Password[16];
uint8_t Timestamp[6];
int Expire_time; 
}message_videosource;  

typedef struct _message_sensor {        //传感器数据上报结构体
message_h message_head;
uint8_t Equip_Id[16];                   //设备ID               
int16_t Data;                           //温度/湿度/空气质量
uint8_t Type;                           //0：温度；1：湿度；2：空气质量
uint32_t SessionId;
}message_sensor;     
                    

#define SERV_TYPE_RESP     1              //通用报文类型
#define SERV_CODE_RESP     1              //通用报文代码

#define SERV_TYPE_LINK     2              //链路检测
#define SERV_CODE_LOGON    1              //登录
#define SERV_CODE_LOGOFF   2              //退出
#define SERV_CODE_HEATBIT  3              //心跳

#define SERV_TYPE_PSET     3              //参数设置
#define SERV_TYPE_PQRY     4              //参数查询
#define SERV_CODE_BASE     1              //基本信息
#define SERV_CODE_NET      2              //网络参数
#define SERV_CODE_INTF     3              //接口参数
#define SERV_CODE_NTP      4              //时钟参数
#define SERV_CODE_FTP      5              //!!!!!!!
#define SERV_CODE_PKEY     6              //终端密码
#define SERV_CODE_SETINTERVAL    7        //传感器采集周期设置
#define SERV_CODE_VIDEOSOURCE    7        //视频源地址查询
#define SERV_CODE_QRYBT    9              //可见未知蓝牙设备列表查询

#define SERV_TYPE_CTRL      5             //命令控制
#define SERV_CODE_RESET     1             //系统复位
#define SERV_CODE_DATACLR   2             //数据清理
#define SERV_CODE_RESTORE   3             //恢复出厂
#define SERV_CODE_AJUSTTIME 4             //时间校准
#define SERV_CODE_BTCON     5             //连接指定蓝牙设备
#define SERV_CODE_BTDEL     6             //删除指定蓝牙设备
#define SERV_CODE_BTCMD     7             //向指定蓝牙设备发送指令
#define SERV_CODE_FILERENAME   8          //重命名文件
#define SERV_CODE_FILEOP       9          //操作文件
#define SERV_CODE_QRYFILELIST  10         //查询文件列表

#define SERV_TYPE_DQRY      6             //数据查询
#define SERV_CODE_SENS      1             //传感器状态
#define SERV_CODE_VER       2             //版本及状态信息
#define SERV_CODE_TIME      3             //当前日期及时间
#define SERV_CODE_EXTSENS   5             //蓝牙传感设备数据
#define SERV_CODE_REGBT     6        	  //注册蓝牙设备
#define SERV_CODE_BT        7        	  //蓝牙设备上线下线通知

#define SERV_TYPE_FILE      7      	  //文件传输
#define SERV_CODE_FILE_UPREQ       1      //文件上传请求
#define SERV_CODE_FILE_DOWNREQ     2      //文件下载请求
#define SERV_CODE_FILE_DATA        3      //文件传输请求

#define TCP_LOGIN                  1
#define TCP_CONNECT                2
#define TCP_DISCONNECT             3


#endif  /* _COLLECT_H_ */

