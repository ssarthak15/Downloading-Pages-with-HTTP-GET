#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, h;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *url = "";

    char buffer[1024];
    if (argc < 7) {
       fprintf(stderr,"usage %s -h hostname -p port -u URL\n", argv[0]);
       exit(0);
    }

    //Taking arguments with flags: cover all 6 possibilities(there has to be a better strategy)
    if(strcmp(argv[1],"-h") == 0)
    {
        //printf("1\n");
        server = gethostbyname(argv[2]);
        h = 2;
        if(strcmp(argv[3],"-p") == 0)
        {
            //printf("2\n");
            portno = atoi(argv[4]);
            if(strcmp(argv[5],"-u") == 0)
            {
                //printf("3\n");
                url = argv[6]; 
                //printf("after 3\n");
            }
            else
                printf("invalid arguments\n");
        }
        else if(strcmp(argv[3],"-u") == 0)
        {
            url = argv[4];
            if(strcmp(argv[5],"-p") == 0)
               portno = atoi(argv[6]);
            else
                printf("invalid arguments\n");
        }
    }
    else if(strcmp(argv[1],"-p") == 0)
    {
        portno = atoi(argv[2]);
        if(strcmp(argv[3],"-h") == 0)
        {
            server = gethostbyname(argv[4]);
            h = 4;
            if(strcmp(argv[5],"-u") == 0)
               url = argv[6];
            else
                printf("invalid arguments\n");
        }
        else if(strcmp(argv[3],"-u") == 0)
        {
            url = argv[4];
            if(strcmp(argv[5],"-h") == 0)
            {
               server = gethostbyname(argv[6]);
               h = 6;
            }
            else
                printf("invalid arguments\n");
        }
    }
    else if(strcmp(argv[1],"-u") == 0)
    {
        url = argv[2];
        if(strcmp(argv[3],"-h") == 0)
        {
            server = gethostbyname(argv[4]);
            h = 4;
            if(strcmp(argv[5],"-p") == 0)
               portno = atoi(argv[6]);
            else
                printf("invalid arguments\n");
        }
        else if(strcmp(argv[3],"-p") == 0)
        {
            portno = atoi(argv[4]);
            if(strcmp(argv[5],"-h") == 0)
            {
               server = gethostbyname(argv[6]);
               h = 6;
            }
            else
                printf("invalid arguments\n");
        }
    } 


    //printf("yo yo\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    //printf("yoyo2\n");
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    else
        printf("connected to the server at port %d host %s\n", portno, argv[h]);
    
    strncpy(buffer,url, sizeof buffer - 1);
    buffer[sizeof buffer - 1] = '\0'; 
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    else
        printf("sent url: %s\n",url);
    bzero(buffer,1024);
    n = read(sockfd,buffer,1023);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("received filename: %s\n",buffer);
    close(sockfd);
    return 0;
}
