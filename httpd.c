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
#define ISspace(x) isspace((int)(x))
#define SERVER_STRING "Server:Tomato'httpd/0.1.0\r\n"

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

static get_line(int sock,char *buf,int size)
{
	int i = 0;
	char c = '\0';
	int n;
	//把终止条件统一为 \n 标准化buf数组
	while ( (i < size - 1) && (c != '\n')){
		if ((n = recv(sock,&c,1,0)) > 0 ){
			if (c == '\r'){
				n = recv(sock,&c,1,MSG_PEEK);//使用MSG_PEEK标志使下一次读取依然可以得到这次读取的内容，可以认为接受窗口不滑动
				if((n > 0) && (c=='\n'))//从tcp buf中将换行符删除掉
					recv(sock,&c,1,0);
				else
					c = '\n';
			}
			buf[i++] = c;
			
		}else
			c = '\n';
	}
	buf[i] = '\0';
	return i;
}
static void unimplemented(int client)
{
	char buf[1024];

	/*http method 不被支持*/
	sprintf(buf,"HTTP/1.0 501 Method Not Implemented\r\n");
	send(client,buf,strlen(buf),0);
	/*服务器信息*/
	sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}
static void not_found(int client)
{
	char buf[1024];

	/* 404 页面 */
	sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client, buf, strlen(buf), 0);
	 /*服务器信息*/
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "your request because the resource specified\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}
static void headers(int client, const char *filename)
{
	char buf[1024];
	(void)filename;  /* could use filename to determine file type */

	/*正常的 HTTP header */
	strcpy(buf, "HTTP/1.0 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	/*服务器信息*/
	strcpy(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
}
static void cat(int client, FILE *resource)
{
	char buf[1024];
	/*读取文件中的所有数据写到 socket */
	fgets(buf, sizeof(buf), resource);
	while (!feof(resource))
	{
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
}

static serve_file(int client,const char *filename)
{
	FILE *resource  = NULL;
	int numchars = 1;
	char buf[1024];
	/*读取丢弃header*/
	buf[0] = 'A';
	buf[1] = '\0';
	while((numchars > 0) && strcmp("\n",buf))
		numchars = get_line(client,buf,sizeof(buf));
	/*打开server的文件*/
	resource = fopen(filename,"r");
	if(resource == NULL)
		not_found(client);
	else{
		/*写HTTP header*/
		headers(client,filename);
		cat(client,resource);//复制文件
	}
	fclose(resource);
}
void accept_request(int client)
{
	char buf[1024];
	int numchars;
	char method[255];
	char url[255];
	char path[512];
	size_t i,j;
	struct stat st;
	int cgi = 0;
	char *query_string = NULL;
	//eg:POST / HTTP/1.1  GET /books/?sex=man&name=Professional HTTP/1.1
	numchars = get_line(client,buf,sizeof(buf));//得到请求的第一行
	i = j = 0;
	//把客户端请求方法存放到method数组
	while(!ISspace(buf[i]) && (i < sizeof(method) -1))
		method[i++]=buf[j++];
	method[i] = '\0';
	/*strcasecmp 判断字符串是否相当并且忽略大小写*/
	if(strcasecmp(method,"GET") && strcasecmp(method,"POST")){
		unimplemented(client);
		printf("not get and post\n");
		return;
	}
	/*开启cgi*/
	if(strcasecmp(method,"POST") == 0)
		cgi = 1;

	/*读取url*/
	i = 0;
	while(ISspace(buf[j]) && (j < sizeof(buf)))
		j++;
	while(!ISspace(buf[j]) && (i<sizeof(url)-1) && (j < sizeof(buf)))
		url[i++] = buf[j++];//存放url
	url[i] = '\0';
	
	/*处理GET 方法*/
	if(strcasecmp(method,"GET")== 0 ){
		query_string = url;//待处理的请求url
		while((*query_string != '?')  && (*query_string != '\0'))
			query_string++;
		/*GET 方法的特点,?后面为参数*/
		if(*query_string == '?'){
			cgi = 1;//开启cgi
			*query_string = '\0';
			query_string++;
		}
	}

	/*格式化url到path数组,html文件都存放在htdocs中*/
	sprintf(path,"htdocs%s",url);
	/*默认情况下为index.html*/
	if(path[strlen(path) - 1] == '/')
		strcat(path,"index.html");
	if(stat(path,&st) == -1){//获取文件的信息
		//未找到对应的文件,把所有header的信息丢掉
		while ((numchars > 0) && strcmp("\n",buf)) /*read & discard headers*/
			numchars = get_line(client,buf,sizeof(buf));
		not_found(client);
	}else{
		/*如果有这个目录，则默认使用该目录下的index.html*/
		if((st.st_mode & S_IFMT) == S_IFDIR)
			strcat(path,"index.html");
		if((st.st_mode & S_IXUSR /*文件所有则有可执行权限*/) || (st.st_mode & S_IXGRP /*用户组具可执行权限*/) || (st.st_mode & S_IXOTH/*其他用户具可执行权限*/))
			cgi = 1;
		//if(!cgi)
			serve_file(client,path);
		//else
		//	execute_cgi(cl)
	}
	close(client);
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
