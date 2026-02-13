#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define RFC868_TO_UNIX 2208988800U

int main(int argc, char *argv[])
{
    if(argc != 3)   //kontrollerar att användare skickar med ip och port
    {
        fprintf(stderr, "Usage: %s <server_ip> <port>" , argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);    //skapar udp socket
    if(sockfd < 0)
    {
        perror("socket");
        close(sockfd);
        return 1;
    }

    //klients adress
    struct sockaddr_in servaddr;
    memset(&servaddr, 0 , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons((uint16_t)port);

    if(inet_pton(AF_INET, server_ip, &servaddr.sin_addr) != 1)  //konverterar ip-strängen till binärt
    {
        fprintf(stderr, "Invalid IP-adress\n");
        return 1;
    }

    if(sendto(sockfd, NULL, 0, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)  //skickar tom request
    {
        perror("sendto");
        close(sockfd);
        return 1;
    }

    // ta emot 4 bytes
    uint32_t net_time = 0;
    socklen_t slen = sizeof(servaddr);
    ssize_t n = recvfrom(sockfd, &net_time, sizeof(net_time), 0, (struct sockaddr *)&servaddr, &slen);

    if(n < 0)
    {
        perror("recvfrom");
        fprintf(stderr, "No reply\n");
        close(sockfd);
        return 1;
    }

    if(n != (ssize_t)sizeof(net_time)) // kontrollerar så vi fått exakt rätt storlek
    {
        fprintf(stderr, "Unexpected reply size: %zd bytes\n", n);
        close(sockfd);
        return 1;
    }

    uint32_t rfc_time = ntohl(net_time);

    if(rfc_time < RFC868_TO_UNIX)
    {
        fprintf(stderr, "Server time is before 1970\n");
        close(sockfd);
        return 1;
    }

    time_t unix_time = (time_t)(rfc_time - RFC868_TO_UNIX); //konvertara till unix tid (sekunder sedan 1970)
    
    printf("Human time: %s\n" , ctime(&unix_time));


    close(sockfd);
    return 0;




}