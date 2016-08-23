#ifndef _SET_H_
#define _SET_H_

#include <stdint.h>

#define buflen  200 

void process_conn_server(int client_fd);

void set_base(int client_fd);
void set_net(int client_fd);
void set_intf(int client_fd);
void set_ntp(int client_fd);
void set_pkey(int client_fd);
void set_interval(int client_fd);
void query_base(int client_fd);
void query_net(int client_fd);
void query_intf(int client_fd);
void query_ntp(int client_fd);
void query_pkey(int client_fd);
void query_videosource(int client_fd);
void query_bluet(int client_fd);
void ctrl_reset(int client_fd);
void ctrl_dataclr(int client_fd);
void ctrl_restore(int client_fd);
void ctrl_ajusttime(int client_fd);
void ctrl_btcon(int client_fd);
void ctrl_btdel(int client_fd); 
void ctrl_btcmd(int client_fd);
void ctrl_filerename(int client_fd);
void ctrl_fileop(int client_fd);
void ctrl_qryfilelist(int client_fd); 
void query_ver(int client_fd); 
void query_time(int client_fd);

#endif  /* _SET_H_ */
