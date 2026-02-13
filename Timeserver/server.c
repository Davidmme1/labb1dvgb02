#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define BUF_SIZE 1024
#define PORT 3737
#define RFC868_TO_UNIX 2208988800U

int main()
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cli_len = sizeof(cliaddr);
    char buffer[BUF_SIZE];

    if(sockfd = socket(AF_INET, SOCK_DGRAM, 0) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }    

    //serverns address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;

    //kopplar socket till port 3737
    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting on UDP port %d...\n", PORT);
    fflush(stdout);

    while(1)
    {
    //inväntar request
    recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &cli_len);

    
    printf("Got request\n");

    time_t unix_time = time(NULL);  //hämta aktuell UNIX tid
    uint32_t rfc868_time = (uint32_t)unix_time + RFC868_TO_UNIX; //konverterar till rfc868 tid   
    uint32_t net_time = htonl(rfc868_time); // gör om tal till rätt format för att skickas

    sendto(sockfd, &net_time, sizeof(net_time), 0, (struct sockaddr *)&cliaddr, cli_len);   //skickar tid

    printf("Sent time\n");
    
    }

    close(sockfd);
    return 0;

}