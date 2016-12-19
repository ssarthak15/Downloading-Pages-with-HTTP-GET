/*
*   Following websites were refered while writing this code:
*   
*   http://www.programminglogic.com/example-of-client-server-program-in-c-using-sockets-and-tcp/
*   http://stackoverflow.com/questions/11254037/how-to-know-when-the-http-headers-part-is-ended
*   http://www.tutorialspoint.com/unix_sockets/socket_server_example.htm
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>



int f_exists(const char *fname)
{
    FILE *file;
    if(file = fopen(fname, "r"))
    {
        fclose(file);
        return 1;
    }
    return 0;
}


void parse_txt_headers(int s, FILE * fp)
{
    int       isnheader = 0;
    ssize_t   readed,fsize = 0;
    char buffer[1024] ="";
    char *content="";


    while((readed = read(s, buffer, 1024)) > 0) 
    {
        fsize += readed;
        if(isnheader == 0)
        {
            isnheader = 1;
            int i;
            for(i =0; i < strlen(buffer)-4; ++i)
            {
                if(buffer[i]=='\r' && buffer[i+1]=='\n' && buffer[i+2]=='\r' && buffer[i+3]=='\n')
                    //memmove(buffer, buffer+i+4, strlen(buffer)-i-4);
                    content = buffer+i+4;
            }

        }
        else
            content = buffer;

        fwrite(content, 1, strlen(content), fp);
        bzero(buffer,1024);
        bzero(content,strlen(content));

    }

    printf("\ndownloaded number of bytes: %u\n", fsize);
}

void parse_http_headers(int s, FILE * fp)
{
   int       isnheader;
   ssize_t   readed=0, fsize=0;
   size_t    len;
   size_t    offset=0;
   size_t    pos=0;
   char      buffer[1024]="";
   char    * eol=""; // end of line
   char    * bol=""; // beginning of line

   isnheader = 0;
   len       = 0;

   // read next chunk from socket
   while((readed = read(s, &buffer[len], (1023-len))) > 0)
   {
        fsize += readed;
      // write rest of data to FILE stream
      if (isnheader != 0)
      {
         fwrite(buffer, 1, readed, fp);
         bzero(buffer, 1024);
      }

      // process headers
      if (isnheader == 0)
      {
         // calculate combined length of unprocessed data and new data
         len += readed;

         // NULL terminate buffer for string functions
         buffer[len] = '\0';

         // checks if the header break happened to be the first line of the
         // buffer
         if (!(strncmp(buffer, "\r\n", 2)))
         {
            if (len > 2)
               fwrite(buffer, 1, (len-2), fp);
            bzero(buffer, 1024);
            continue;
         };
         if (!(strncmp(buffer, "\n", 1)))
         {
            if (len > 1)
               fwrite(buffer, 1, (len-1), fp);
            bzero(buffer, 1024);
            continue;
         }

         // process each line in buffer looking for header break
         bol = buffer;
         while((eol = index(bol, '\n')) != NULL)
         {
            // update bol based upon the value of eol
            bol = eol + 1; 

            // test if end of headers has been reached
            if ( (!(strncmp(bol, "\r\n", 2))) || (!(strncmp(bol, "\n", 1))) )
            {
               // note that end of headers has been reached
               isnheader = 1;

               // update the value of bol to reflect the beginning of the line
               // immediately after the headers
               if (bol[0] != '\n')
                  bol += 1;
               bol += 1;

               // calculate the amount of data remaining in the buffer
               len = len - (bol - buffer);

               // write remaining data to FILE stream
               if (len > 0)
               {
                  fwrite(bol, 1, len, fp);
                  bzero(bol, len);
               }


               // reset length of left over data to zero and continue processing
               // non-header information
               len = 0;
            }
         }

         if (isnheader == 0)
         { 
            // shift data remaining in buffer to beginning of buffer
            offset = (bol - buffer);
            for(pos = 0; pos < offset; pos++)
               buffer[pos] = buffer[offset + pos];

            // save amount of unprocessed data remaining in buffer
            len = offset;
         }
      }
   }
   printf("\ndownloaded number of bytes: %u\n", fsize);
   return;
}


char *Get_Page(char url[512])
{
    
    int socket_desc, i;
    char ip[100]= "", request[100]="";
    char *hostname = url;
    struct sockaddr_in server;
    struct hostent *he;
    struct in_addr **addr_list;
    FILE *fp;

    //printf("before 1\n");
    char *arg="";
    arg = url;
    //printf("1\n");
    char firstHalf[500]="";
    char secondHalf[500]="";
    //printf("after 1\n");

    //append '/' in the end if not present
    if(arg[strlen(arg)-1] != '/')
    {
        strcat(arg, "/");
    }

    //remove http:// if present
    if(arg[0] == 'h' && arg[1]== 't' && arg[2]== 't' && arg[3]== 'p' && arg[4]== ':' && arg[5]== '/' &&  arg[6]== '/')
    {
        memmove(arg, arg+7, strlen(arg)-7);
        arg[strlen(arg)-7] = '\0';
    }

    //remove https:// if present
    if(arg[0] == 'h' && arg[1]== 't' && arg[2]== 't' && arg[3]== 'p' && arg[4]== 's' && arg[5]== ':' &&  arg[6]== '/' && arg[7] == '/')
    {
        memmove(arg, arg+8, strlen(arg)-8);
        arg[strlen(arg)-8] = '\0';
    }

    //extract hostname
    int j;
    //printf("\n%d\n",strlen(arg));
    for (j = 0; j < strlen(arg); j++)
    {
        //printf("%d\n",i);
        if (arg[j] == '/')
        {
                strncpy(firstHalf, arg, j);
                firstHalf[j] = '\0';
                break;
        }
    }

    //extract page(second half)
    if(j < strlen(arg))
    {
        strcat(secondHalf, &arg[j]);
    }

    //if requested page is a file like .txt or .pdf or .jpg (check for '.') remove the '/'
    if(secondHalf[strlen(secondHalf)-5] == '.')
        secondHalf[strlen(secondHalf)-1] = '\0';
    else if(secondHalf[strlen(secondHalf)-1] != '/')
    {
        strcat(secondHalf, "/");
    }


    hostname = firstHalf;
    //printf("%s\n", hostname);


    //printf("3\n");
    if ((he = gethostbyname(hostname)) == NULL) {
        //gethostbyname failed
        herror("gethostbyname\n");
        return;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++) {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
    }

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket!\n");
    }

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0) {
        printf("connect error!\n");
        return;
    }

    //printf("Connected...\n");

    //Send some data
    snprintf(request, 99, "GET /%s HTTP/1.0\r\n"
            "Host: %s\r\n"
                "\r\n\r\n", secondHalf, hostname
    );

    if (send(socket_desc, request, strlen(request), 0) < 0) 
    {
        puts("Send failed!\n");
        return;
    }
    //puts("Data Sent...\n");

    //Receive a reply from the server

    static char command[512];
    snprintf(command, 511, "echo -n '%s' | md5sum", url);
    FILE* file = popen(command,"r");
    static char dir_file[512];
    if(fgets(dir_file, 512, file))
    {
        int j;
        for(j = 0; j < strlen(dir_file); j++)
        {
            if(dir_file[j] == ' ')
                dir_file[j] = '\0';
        }
        fp = fopen( dir_file, "wb+");
            pclose(file);
    }
    else
        printf("ERROR\n");

    printf("downloaded the page\n");
    printf("storing to filename: %s", dir_file);

    int slen = strlen(secondHalf);
    if(secondHalf[slen-1] == 't' && secondHalf[slen-2] == 'x' && secondHalf[slen-3] == 't' && secondHalf[slen-4] == '.')
        parse_txt_headers(socket_desc, fp);
    else
        parse_http_headers(socket_desc, fp);


    fclose(fp);
    close(socket_desc);
    return dir_file;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}



void chprocess(int newsockfd) {
    int n;
    char buffer[1024], *recfile;
    bzero(buffer,1024);
    n = read(newsockfd,buffer,1023);
    if (n < 0) 
        error("ERROR reading from socket");
    printf("a client has connected\n");
    printf("received url: %s\n",buffer);
    
    recfile = Get_Page(buffer);


    n = write(newsockfd,recfile, strlen(recfile));
    if (n < 0) 
        error("ERROR writing to socket");

    printf("sent filename to the client: %s\n", recfile);
    if (n < 0) error("ERROR writing to socket");
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int pid;

     if (argc < 3) 
     {
        fprintf(stderr,"ERROR! Improper Format\nusage %s -p port\n",argv[0]);
        exit(1);
     }


     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     if((listen(sockfd,5)) == -1)
	error("listen");

    printf("waiting for clients\n");

     while(1)
     {
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if(newsockfd  < 0)
          error("ERROR on accept");
	pid = fork();

    if (pid < 0) 
    {
         error("ERROR on fork");
    }

    if (pid == 0) 
    {
         // the client process
         close(sockfd);
         chprocess(newsockfd);
         exit(0);
    }
        
   } // end of while 

     close(sockfd);
     return 0; 
}

