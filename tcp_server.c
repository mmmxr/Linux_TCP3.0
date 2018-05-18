#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef struct {
    int sock;
    char ip[24];
    int port;
}net_info_t;

int StartUp(char* ip,int port)
{
    //建立套接字
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        printf("sock error\n");
        exit(2);
    }
    //填充信息
    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_addr.s_addr=inet_addr(ip);
    local.sin_port=htons(port);

    //绑定
    if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
    {
        printf("bind error\n");
        exit(3);
    }
    //监听
    if(listen(sock,5)<0)
    {
        printf("listen error\n");
        exit(4);
    }

    return sock;
}
void service(int sock,char* ip,int port)
{
    char buf[64];
    while(1)
    {
        buf[0]=0;
        size_t s=read(sock,&buf,sizeof(buf));
        if(s>0)
        {
            buf[s]=0;
            printf("%s:%d say# %s\n",ip,port,buf);
            write(sock,buf,strlen(buf));
        }
        else if(s==0)
        {
            printf("client %s quit!\n",ip);
            break;
        }
        else
        {
            printf("read error!\n");
            break;
        }
    }

}
void* thread_service(void *arg)
{
    net_info_t* p_thread=(net_info_t*)arg;
    service(p_thread->sock,p_thread->ip,p_thread->port);
    close(p_thread->sock);
    free(p_thread);
    return 0;
}
int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        printf("Usage:%s:ip port\n",argv[0]);
        exit(1);
    }
    int listen_sock=StartUp(argv[1],atoi(argv[2]));
    struct sockaddr_in peer;
    char ipbuf[64];
    while(1)
    {
        ipbuf[0]=0;
        socklen_t len=sizeof(peer);
        int new_sock=accept(listen_sock,(struct sockaddr*)&peer,&len);
        if(new_sock<0)
        {
            printf("accept error\n");
            continue;
        }
        inet_ntop(AF_INET,(const void* )&peer.sin_addr,ipbuf,sizeof(ipbuf));
        int p=ntohs(peer.sin_port);
        printf("get a new connection:[%s:%d]\n",ipbuf,p);

        net_info_t *info_thread=(net_info_t*)malloc(sizeof(net_info_t));
        if(info_thread==NULL)
        {
            perror("malloc error");
            close(new_sock);
            continue;
        }
        info_thread->sock=new_sock;
        strcpy(info_thread->ip,ipbuf);
        info_thread->port=p;

        pthread_t tid;
        pthread_create(&tid,NULL,thread_service,(void*)info_thread);
        pthread_detach(tid);

    }
    return 0;
}
