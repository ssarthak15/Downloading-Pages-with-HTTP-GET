





Computer Networks                                    Sarthak Sharma
Request for Comments:                                University of Houston
Obsoletes: None                                      11 September 2016
Updates:  None


                DownURL: A page downloading program

Abstract

        This is a socket client and socket server program. The client sends 
        a URL to the server, the server fetches the URL, stores it in the 
        current directory and sends the name of the file to the client.

Description

        The program is divided into two files: Server(sserver) and Client(sclient). 

        sserver supports a flag -p with which you can specify the port on which it 
        listens for incoming connections. The server can service multiple clients 
        at the same time. It uses forks to accomplpish this. The server opens a 
        socket to the webserver, downloads the content, and saves it to a file. Note 
        that the content might be any kind of binary data like html pages, jpeg images,
         .txt files, .pdf files, .zip files, .php pages etc.

        It uses a simple socket program to connect to the webserver on port 80 and 
        downloads the content using a GET request. The response contains HTTP header 
        which is removed before storing the file.


        Below is the sample output for the server program:
      -------------------------------------------------------------
      |  ./sserver -p <port no>                                   |
      |  waiting for clients                                      |
      |  a client has connected                                   |
      |  received url: xxxxx                                      |
      |  downloaded the pages                                     |
      |  storing to filename: yyyyy                               |
      |  downloaded number of bytes: zzzzz                        |
      |  sent filename to the client: yyyyy                       |
      -------------------------------------------------------------


         sclient supports three flags -p, -h and -u with which you can specify the 
         port, hostname and the URL respectively for the server to which you want to 
         connect. THE -u flag enables the user to specify the URL for the page to be 
         downloaded and stored by the server.


         Below is the sample output for the client program:
      -----------------------------------------------------------------
      |  ./sclient -p 5000 -h bayou.cs.uh.edu -u AAAAA                |
      |  connected to the server at port 5000 host bayou.cs.uh.edu    |
      |  sent url: xxxxx                                              |
      |  received filename: yyyyy                                     |
      -----------------------------------------------------------------
    
General Architecture
                      
                                                          |
      +---------+               +----------+              |  +--------+
      |         | request + url |          | connect+GET  |  |        |
      | sclient |-------------->| sserver  |--------------|->|  Web   |
      | program |               | program  |              |  |        |
      |         |<--------------|          |<-------------|--| Server |
      |         |    filename   |          |  Header+data |  |        |
      +---------+               +----------+              |  +--------+
                                  ^     |                 |              
                      data(file)  |     | Header+data     |
                                  |     v                 |
                              ++++++++++++++++            |
                              |remove headers|            |
                              ++++++++++++++++            |

Test cases

      -----------------------------------------------------------------
      |  ./ssever -p 51717                                            |
      |  ./sclient -p 51717 -h localhost -u www.uh.edu/               |
      |                                                               |
      |  You can find more test cases here                            |
      |                                                               |
      |  www.uh.edu/landing-page-images/page-header-news-events.jpg   |
      |  www.uh.edu/academics/forms/transcript-request-form-2013.pdf  |
      |  www.cs.uh.edu/                                               |
      |  www.gnu.org/software/gnugo/README.txt                        |
      -----------------------------------------------------------------
