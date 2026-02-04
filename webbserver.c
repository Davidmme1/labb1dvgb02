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

int main()
{
    int server_fd, client_fd, fd, bytes;
    char buffer[BUF_SIZE];
    struct sockaddr_in server_addr;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    //nollar och initierar socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    int on = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    if(bind(server_fd, (struct sockaddr *)&server_addr , sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, QUEUE_SIZE) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        client_fd = accept(server_fd, NULL, NULL);  //client connected

        bytes = read(client_fd, buffer, sizeof(buffer) - 1);
        buffer[bytes] = '\0';

        char method[10], path[100], protocol[10];
        sscanf(buffer, "%s %s %s" , method, path, protocol);

        char file_name[256];
        sscanf(buffer, "GET /%s", file_name);

        char filepath[128];
        if(strcmp(path, "/") == 0)
        {
        strcpy(path, "/index.html");
        }
        
        snprintf(filepath, sizeof(filepath), "sample_website%s" , path);
        printf("path=%s  filepath=%s\n", path, filepath);

        fd = open(filepath, O_RDONLY);
    
        if(fd < 0)
        {
            perror("Could not open file");
            char not_found[] = "HTTP/1.1 404 Not Found\r\n" "Content-Length: \r\n" "Content-Type: \r\n";
            write(client_fd, not_found, strlen(not_found));
        }
        else
        {
            int file_size = lseek(fd, 0, SEEK_END);
            lseek(fd, 0 ,SEEK_SET);

            // bestämmer vilken Content-Type
            char *ext = strrchr(path, '.');
            char *mime_type = "application/octet-stream";

            if(ext != NULL)
            {
                if(strstr(ext, ".html"))
                {
                    mime_type = "text/html";
                }
                else if(strcmp(ext, ".jpeg") || strcmp(ext, ".jpg") == 0)
                {
                    mime_type = "image/jpeg";
                }
                else if(strcmp(ext, ".png"))
                {
                    mime_type = "image/png";
                }
                else if(strcmp(ext, ".css"))
                {
                    mime_type = "text/css";
                }
                else if(strcmp(ext, ".js"))
                {
                    mime_type = "application/javascript";
                }
            }


            char found[BUF_SIZE];
            sprintf(found, "HTTP/1.1 200 OK\r\n" "Content-Length: %d\r\n" "Content-Type: %s\r\n\r\n" , file_size, mime_type);
            write(client_fd, found, strlen(found));
        
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