Part 1 Webserver

How to run:

In terminal write:
1.   gcc webserver.c -o webserver

2.  ./webserver 

In webbrowser search:
    http://localhost:8080/

How to run with web_check.py:

In terminal write:
1. chmod +x web_check.py
2. Compile file: gcc webserver -o webserver
3. ./web_check.py ./webserver

Part 2 Time Server

How to run:

To run this program you need to open 2 terminals

1. In terminal 1 write:
    gcc server.c -o server
    ./server

2. In terminal 2 write:
    gcc client.c -o client
    ./client 127.0.0.1 3737



