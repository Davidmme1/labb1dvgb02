#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define SERVER_PORT 8080
#define QUEUE_SIZE 5
#define BUF_SIZE 4096
#define PATH_SIZE 512

int main()
{
    int server_fd, client_fd, fd, bytes;
    char buffer[BUF_SIZE];
    struct sockaddr_in server_addr;
    
    // skapar TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    //initierar socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    //tilåter att servern startas om snabbt utan att error uppstår
    int on = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    //kopplar upp socket till port
    if(bind(server_fd, (struct sockaddr *)&server_addr , sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    //sätter socket i lyssningsläge
    if(listen(server_fd, QUEUE_SIZE) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        client_fd = accept(server_fd, NULL, NULL);  //client connected

        //läser HTTP-request
        bytes = read(client_fd, buffer, sizeof(buffer) - 1);
        if(bytes <= 0)
        {
            close(client_fd);
            continue;
        }

        buffer[bytes] = '\0'; // gör om till sträng

        char method[10], path[100], protocol[10];   //plockar ut metod path och protokoll ur första raden i request
        int parsed = sscanf(buffer, "%s %s %s" , method, path, protocol);
        if(parsed != 3)
        {
            close(client_fd);   //om något inte stämde i parsning, stäng klient vänta på ny
            continue;
        }


        if(strcmp(path, "/") == 0)  // vi request till / så skickas webbläsare till startsida
        {
        strcpy(path, "/index.html");
        }

        char filepath[PATH_SIZE];
        snprintf(filepath, sizeof(filepath), "%s" , path + 1); // hoppar över / i början av path för att öppna fil direkt

        fd = open(filepath, O_RDONLY);
    
        if(fd < 0) //404 - response
        {
            perror("Could not open file");

            const char body[] = "<h1>404 Not Found<h1>";
            char not_found[256];
            snprintf(not_found, sizeof(not_found), "HTTP/1.1 404 Not Found\r\n" "Content-Length: %d\r\n" "Content-Type: text/html\r\n" "\r\n" , (int)(strlen(body) - 1));
            write(client_fd, not_found, strlen(not_found));
            write(client_fd, body, strlen(body) - 1);
        }
        else
        {
            int file_size = lseek(fd, 0, SEEK_END); // storlek på fil
            lseek(fd, 0 ,SEEK_SET); 

            // bestämmer vilken Content-Type
            const char *ext = strrchr(path, '.');
            const char *mime_type = "application/octet-stream"; //standard

            if(ext)
            {
                if(strcmp(ext, ".html") == 0)
                {
                    mime_type = "text/html";
                }
                else if(strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".jpg") == 0)
                {
                    mime_type = "image/jpeg";
                }
                else if(strcmp(ext, ".png") == 0)
                {
                    mime_type = "image/png";
                }
                else if(strcmp(ext, ".css") == 0)
                {
                    mime_type = "text/css";
                }
                else if(strcmp(ext, ".js") == 0)
                {
                    mime_type = "application/javascript";
                }
            }

            //skickar 200 OK response

            char found[BUF_SIZE];
            sprintf(found, "HTTP/1.1 200 OK\r\n" "Content-Length: %d\r\n" "Content-Type: %s\r\n\r\n" , file_size, mime_type);
            write(client_fd, found, strlen(found));
        
            //skickar fil
            while(1)
            {
                bytes = read(fd, buffer, BUF_SIZE);
                if(bytes <= 0) break;
                write(client_fd, buffer, bytes);
            }
                    close(fd);
        }
        close(client_fd);

    }

    close(server_fd);
    return 0;

}