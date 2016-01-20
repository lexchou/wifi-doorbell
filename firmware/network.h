#ifndef NETWORK_H
#define NETWORK_H


void rak410_write(const char*);
void rak410_dhcp();
void rak410_reset();
void rak410_scan();
void rak410_connect(const char* ssid, const char* password);
void rak410_http_get(const char* url);
void rak410_rssi();
int rak410_tcp(const char* dest_ip, int dest_port);
void rak410_close(int socket);
void rak410_send(int socket, int len, const char* data);




#endif//NETWORK_H
