#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

static void error_die(const char *sc)
{
	perror(sc);//将错误原因和sc字符串输出
	exit(1);
}

static int startup(unsigned short *port)
{
	int httpd = -1;
	struct sockaddr_in name;
	
	if( (httpd = socket(PF_INET,SOCK_STREAM,0)) < 0)
		error_die("socket");
	memset(&name,0,sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
		error_die("bind");
	if(*port == 0) {
		int namelen = sizeof(name);
		if(getsockname(httpd,(struct sockaddr *)&name,&namelen) == -1)
			error_die("getsockname");
		*port = ntohs(name.sin_port);
	}
	if(listen(httpd,5) < 0)
		error_die("listen");
	return httpd;
}
void accept_request(int client)
{
	//1.test 
	char buf[1024] = {'\0'};
	int length = recv(client,buf,1024,0);
	if (length < 0)
		error_die("recv");

	printf("recv:%s\n",buf);
}
int main(int argc,char *argv[])
{
	int server_sock = -1;
	unsigned short port = 8090;
	int client_sock = -1;
	struct sockaddr_in client_name;
	int client_name_len = sizeof(client_name);
	pthread_t newthread;

	server_sock = startup(&port);
	while(1){
		if ( (client_sock = accept(server_sock,(struct sockaddr *)&client_name,&client_name_len)) < 0)
			error_die("accept");
		if ( pthread_create(&newthread, NULL, accept_request, client_sock) != 0)
			perror("phread_create");
	
	}
	close(server_sock);
	return 0;
}
