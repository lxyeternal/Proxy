#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>   
#include <sys/socket.h>  
#include "csapp.h"


#define MAX_MALLOC 10



static const char *user_agent_str = "User-Agent: Mozilla/5.0 \
(X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_str = "Accept: text/html,application/xhtml+xml,\
application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_str = "Accept-Encoding: \
gzip, deflate\r\n";
static const char *connection_str = "Connection: close\r\n";
static const char *proxy_connection_str = "Proxy-Connection: close\r\n";
static const char *http_version_str = "HTTP/1.0\r\n";
/* client_bad_request_str used when host is invalid */
static const char *client_bad_request_str = "HTTP/1.1 400 \
Bad Request\r\nServer: Apache\r\nContent-Length: 140\r\nConnection: \
close\r\nContent-Type: text/html\r\n\r\n<html><head></head><body><p>\
This webpage is not available, because DNS lookup failed.</p></body></html>";


void job(void *arg);
int forward_to_server(int fd, int *to_server_fd);
int forward_to_client(int to_client_fd, int to_server_fd);

/* helper functions */
int parse_request_line(char *buf, char *method, char *protocol,
                       char *host_port, char *resource, char *version);
void parse_host_port(char *host_port, char *remote_host, char *remote_port);
void get_size(char *buf, unsigned int *size_pointer);
void close_fd(int *to_client_fd, int *to_server_fd);




int main (int argc, char *argv []) {
    int listenfd, *connfd, clientlen, port = 0;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    // ignore SIGPIPE
    Signal(SIGPIPE, SIG_IGN);

    // check inputs
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);
    if (port == 0) {
        fprintf(stderr, "please enter valid port number\n");
        exit(1);
    }

    listenfd = Open_listenfd(port);
    if (listenfd < 0) {
        fprintf(stderr, "cannot listen to port: %d\n", port);
        exit(1);
    }

    while (1) {
        clientlen = sizeof(clientaddr);
        int count = 0;
        // use MAX_MALLOC to limit max tries
        while ((connfd = (int *)malloc(sizeof(int))) == NULL) {
            if (count > MAX_MALLOC) {
                break;
            }
            count++;
            sleep(1);
        }
        *connfd = Accept(listenfd, (SA *)&clientaddr,
                         (socklen_t *)&clientlen);
        Pthread_create(&tid, NULL, (void *)job, (void *)connfd);
    }
    return 0;
}


void job(void *arg) {
    Pthread_detach(pthread_self());
    // get file descriptor and free the pointer
    int to_client_fd = *(int *)arg;
    Free(arg);

    int to_server_fd = -1;
    int rc = 0;

    rc = forward_to_server(to_client_fd, &to_server_fd);
    if (rc == -1) {
        // some error
        close_fd(&to_client_fd, &to_server_fd);
        Pthread_exit(NULL);
    } else{
        if (forward_to_client(to_client_fd, to_server_fd) == -1) {
            close_fd(&to_client_fd, &to_server_fd);
            Pthread_exit(NULL);
        }
    }
    close_fd(&to_client_fd, &to_server_fd);
    return;
}



int forward_to_server(int fd, int *to_server_fd) {
    char buf[MAXLINE], request_buf[MAXLINE];
    char method[MAXLINE], protocol[MAXLINE];
    char host_port[MAXLINE];
    char remote_host[MAXLINE], remote_port[MAXLINE], resource[MAXLINE];
    char version[MAXLINE];
    char origin_request_line[MAXLINE];
    char origin_host_header[MAXLINE];
    char url[MAXLINE]={0};
    char *ip[100]={0};
	char *www[100]={0};
	int i;
	char **pptr;   
    struct hostent *hptr;   
    char str[32];   
    char dest_ip[32];
    int flag=1;
    
    int has_user_agent_str = 0, has_accept_str = 0,
        has_accept_encoding_str = 0,
        has_connection_str = 0, has_proxy_connection_str = 0,
        has_host_str = 0;

    rio_t rio_client;

    strcpy(remote_host, "");
    strcpy(remote_port, "80");

    Rio_readinitb(&rio_client, fd);
    if (Rio_readlineb(&rio_client, buf, MAXLINE) == -1) {
        return -1;
    }
    // used incase dns lookup failed
    strcpy(origin_request_line, buf);

    if (parse_request_line(buf, method, protocol, host_port,
                           resource, version) == -1) {
        return -1;
    }

	strcpy(url,protocol);
	strcat(url,"://");
	strcat(url,host_port);
	strcat(url,resource);
	printf("*********%s\n",url);
	
	
	for(i=0;i<100;i++)
	{
		if(!www[i])
		{
			break;
		}
		if(strstr(url,www[i]))
		{ 
			return -1;
		}
	}
	
	
    parse_host_port(host_port, remote_host, remote_port);



    if ((hptr = gethostbyname(remote_host)) == NULL)   
    {   
        printf("error\n");   
        return -1;
    }  
    
	pptr=hptr->h_addr_list;
    strcpy(dest_ip,(char*)(inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str))));
    printf(" address:%s\n", dest_ip);
    
    for(i=0;i<100;i++)
	{
		if(!ip[i])
		{
			break;
		}
		if(strstr(dest_ip,ip[i]))
		{
			return -1;
		}
	}
	

    if (strstr(method, "GET") != NULL) {
        // GET method

        // compose our request line
        strcpy(request_buf, method);
        strcat(request_buf, " ");
        strcat(request_buf, resource);
        strcat(request_buf, " ");
        strcat(request_buf, http_version_str);

        // process request header
        while (Rio_readlineb(&rio_client, buf, MAXLINE) != 0) {
            if (strcmp(buf, "\r\n") == 0) {
                break;
            } else if (strstr(buf, "User-Agent:") != NULL) {
                strcat(request_buf, user_agent_str);
                has_user_agent_str = 1;
            } else if (strstr(buf, "Accept-Encoding:") != NULL) {
                strcat(request_buf, accept_encoding_str);
                has_accept_encoding_str = 1;
            } else if (strstr(buf, "Accept:") != NULL) {
                strcat(request_buf, accept_str);
                has_accept_str = 1;
            } else if (strstr(buf, "Connection:") != NULL) {
                strcat(request_buf, connection_str);
                has_connection_str = 1;
            } else if (strstr(buf, "Proxy Connection:") != NULL) {
                strcat(request_buf, proxy_connection_str);
                has_proxy_connection_str = 1;
            } else if (strstr(buf, "Host:") != NULL) {
                strcpy(origin_host_header, buf);
                if (strlen(remote_host) < 1) {
                    sscanf(buf, "Host: %s", host_port);
                    parse_host_port(host_port, remote_host, remote_port);
                }
                strcat(request_buf, buf);
                has_host_str = 1;
            } else {
                strcat(request_buf, buf);
            }
        }
        // if not sent, copy in out headers
        if (has_user_agent_str != 1) {
            strcat(request_buf, user_agent_str);
        }
        if (has_accept_encoding_str != 1) {
            strcat(request_buf, accept_encoding_str);
        }
        if (has_accept_str != 1) {
            strcat(request_buf, accept_str);
        }
        if (has_connection_str != 1) {
            strcat(request_buf, connection_str);
        }
        if (has_proxy_connection_str != 1) {
            strcat(request_buf, proxy_connection_str);
        }
        if (has_host_str != 1) {
            sprintf(buf, "Host: %s:%s\r\n", remote_host, remote_port);
            strcat(request_buf, buf);
        }
        strcat(request_buf, "\r\n");
        if (strcmp(remote_host, "") == 0) {
            return -1;
        }


        // client to server
        *to_server_fd = Open_clientfd(remote_host, atoi(remote_port),
                                    origin_request_line, origin_host_header);
        if (*to_server_fd == -1) {
            return -1;
        } else if (*to_server_fd == -2) {
            // dns lookup failed, write our response page
            // caused by invalid host
            strcpy(buf, client_bad_request_str);
            Rio_writen(fd, buf, strlen(buf));
            return -1;
        }
        if (Rio_writen(*to_server_fd, request_buf,
                       strlen(request_buf)) == -1) {
            return -1;
        }
        return 0;
    } else {
        // non GET method
        unsigned int length = 0, size = 0;
        strcpy(request_buf, buf);
        while (strcmp(buf, "\r\n") != 0 && strlen(buf) > 0) {
            if (Rio_readlineb(&rio_client, buf, MAXLINE) == -1) {
                return -1;
            }
            if (strstr(buf, "Host:") != NULL) {
                strcpy(origin_host_header, buf);
                if (strlen(remote_host) < 1) {
                    sscanf(buf, "Host: %s", host_port);
                    parse_host_port(host_port, remote_host, remote_port);
                }
            }
            get_size(buf, &size);
            strcat(request_buf, buf);
        }
        if (strcmp(remote_host, "") == 0) {
            return -1;
        }
        *to_server_fd = Open_clientfd(remote_host, atoi(remote_port),
                                    origin_request_line, origin_host_header);
        if (*to_server_fd < 0) {
            return -1;
        }
        // write request line
        if (Rio_writen(*to_server_fd, request_buf,
                       strlen(request_buf)) == -1) {
            return -1;
        }
        // write request body
        while (size > MAXLINE) {
            if ((length = Rio_readnb(&rio_client, buf, MAXLINE)) == -1) {
                return -1;
            }
            if (Rio_writen(*to_server_fd, buf, length) == -1) {
                return -1;
            }
            size -= MAXLINE;
        }
        if (size > 0) {
            if ((length = Rio_readnb(&rio_client, buf, size)) == -1) {
                return -1;
            }
            if (Rio_writen(*to_server_fd, buf, length) == -1) {
                return -1;
            }
        }
        return 2;
    }
}


int forward_to_client(int to_client_fd, int to_server_fd) {
    rio_t rio_server;
    char buf[MAXLINE];
    unsigned int length = 0, size = 0;

    Rio_readinitb(&rio_server, to_server_fd);
    // forward status line
    if (Rio_readlineb(&rio_server, buf, MAXLINE) == -1) {
        return -1;
    }
    if (Rio_writen(to_client_fd, buf, strlen(buf)) == -1) {
        return -1;
    }
    // forward response headers
    while (strcmp(buf, "\r\n") != 0 && strlen(buf) > 0) {
        if (Rio_readlineb(&rio_server, buf, MAXLINE) == -1) {
            return -1;
        }
        get_size(buf, &size);
        if (Rio_writen(to_client_fd, buf, strlen(buf)) == -1) {
            return -1;
        }
    }
    // forward response body
    if (size > 0) {
        while (size > MAXLINE) {
            if ((length = rio_readnb(&rio_server, buf, MAXLINE)) == -1) {
                return -1;
            }
            if (Rio_writen(to_client_fd, buf, length) == -1) {
                return -1;
            }
            size -= MAXLINE;
        }
        if (size > 0) {
            if ((length = Rio_readnb(&rio_server, buf, size)) == -1) {
                return -1;
            }
            if (Rio_writen(to_client_fd, buf, length) == -1) {
                return -1;
            }
        }
    } else {
        while ((length = Rio_readnb(&rio_server, buf, MAXLINE)) > 0) {
            if (Rio_writen(to_client_fd, buf, length) == -1) {
                return -1;
            }
        }
    }
    return 0;
}



int parse_request_line(char *buf, char *method, char *protocol,
                       char *host_port, char *resource, char *version) {
    char url[MAXLINE];
    // check if it is valid buffer
    if (strstr(buf, "/") == NULL || strlen(buf) < 1) {
        return -1;
    }
    // set resource default to '/'
    strcpy(resource, "/");
    sscanf(buf, "%s %s %s", method, url, version);
    if (strstr(url, "://") != NULL) {
        // has protocol
        sscanf(url, "%[^:]://%[^/]%s", protocol, host_port, resource);
    } else {
        // no protocols
        sscanf(url, "%[^/]%s", host_port, resource);
    }
    return 0;
}




/*
 * get_size - get size from content-length header
 */
void get_size(char *buf, unsigned int *size_pointer) {
    if (strstr(buf, "Content-Length")) {
        sscanf(buf, "Content-Length: %d", size_pointer);
    }
}


/*
 * parse_host_port - parse host:port (:port optional)to two parts
 */
void parse_host_port(char *host_port, char *remote_host, char *remote_port) {
    char *tmp = NULL;
    tmp = index(host_port, ':');
    if (tmp != NULL) {
        *tmp = '\0';
        strcpy(remote_port, tmp + 1);
    } else {
        strcpy(remote_port, "80");
    }
    strcpy(remote_host, host_port);
}


void close_fd(int *to_client_fd, int *to_server_fd) {
    if (*to_client_fd >= 0) {
        Close(*to_client_fd);
    }
    if (*to_server_fd >= 0) {
        Close(*to_server_fd);
    }
}
