/*
*	Serial application simulating WiFi interface for 
*	communicating Nucleo f3 MCU and Thingspeak cloud
*
*	authors:	Niccolò Fabrizio Pagano
*				Enrico Verzì
*				
*/



#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

void error(const char *msg) { perror(msg); exit(0); }

int send_to_thingspeak(int channel, float value)
{
    /* first what are we going to send and where are we going to send it? */
    int portno =        80;
    char *host =        "api.thingspeak.com";
    char *message_fmt = "POST /update?key=%s&field%d=%f HTTP/1.0\r\n\r\n";
    char *api = "1Y20XFANH74CJ26U";
    char *token;
    int i;

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[1024],response[4096];

    //if (argc < 3) { puts("Parameters: <apikey> <command>"); exit(0); }

    /* fill in the parameters */
    //sprintf(message,message_fmt,argv[1],argv[2]);
    //printf("Request:\n%s\n",message);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) error("ERROR, no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* send the request */
    sprintf(message, message_fmt, api, channel, value);
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
        bytes = read(sockfd,response+received,total-received);
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    if (received == total)
        error("ERROR storing complete response from socket");

    /* close the socket */
    close(sockfd);

    /* process response */
    //printf("Response:\n%s\n",response);

    token = strtok(response, "\n");
    while(token){
	if(atoi(token) != 0) printf("\nResponse: %d", atoi(token));
        token = strtok(NULL, "\n");
    }

    return 0;
}

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */

int main(void){
	
	char buf [100];
	char aux [100];
	char *token;
	int n, i, channel;
	float value;
	struct termios options;
   	int fd; /* File descriptor for the port */

  	fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
  
	if (fd == -1){
	   /*
	    * Could not open the port.
	    */
	
	    perror("open_port: Unable to open /dev/ttyACM0 - ");
	}
	//else fcntl(fd, F_SETFL, 0); //The read function is set to return immediately
	  
	  
	  
	  
	printf("\nDopo fcntl");  
	  
	  
  

	/*
	 * Get the current options for the port...
	 */
	
	tcgetattr(fd, &options);
	
	/*
	 * Set the baud rates to 19200...
	 */
	
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	
	/*
	 * Enable the receiver and set local mode...
	 */
	
	options.c_cflag |= (CLOCAL | CREAD);
	
	/*
	 * Set the new options for the port...
	 */
	
	tcsetattr(fd, TCSANOW, &options);
	
	
	
	
	
	
	
	//No parity (8N1):
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	
	  
	  
	  
	  
	  
	  
	  
	options.c_cflag |= CRTSCTS;    /* Also called CRTSCTS */
	  
	//oppure options.c_cflag &= ~CNEW_RTSCTS;
	  
	  
	  
	  
	  
	  
	  
	//options.c_lflag |= (ICANON | ECHO | ECHOE); //mod canonica
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //mod raw
	  
	  
	  
	  
	  
	options.c_iflag |= (INPCK | ISTRIP);
	  
	  
	  
	  
	  
	//options.c_iflag |= (IXON | IXOFF | IXANY);   //software flow control
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
	  
	  
	  
	  
	  
	options.c_oflag |= ONLCR;
	  
	  
	printf("\nPrima while 1");
	  
	  
	i = 0;
	while(1){
		n = read (fd, buf, sizeof buf);
		if(n>0){
			printf("\n[%d] %s", i, buf);
			strcpy(aux, buf);
			token = strtok(aux, " ");
			if(token != NULL){
				channel = atoi(token);
				printf("\nChannel: %d ", channel);
				token = strtok(NULL, " ");
				if(token != NULL ){
					value = atof(token);
					printf("Value: %f", value);
					if(value != 0 && channel != 0)
					send_to_thingspeak(channel, value);
				}
			}
			memset(buf, 0, 100);
			i++;
		}/*else{ 
			printf("\n[%d]No data", i);
			sleep(1);
		}*/
	}
  
  
}
