#include "listener.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <gmpxx.h>

void Listen()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = 80;
    
    // Bind socket to address
    if(bind(socketfd, (sockaddr*) &address, sizeof(address)) == -1)
    {
        std::cerr << "bind() failed: " << errno << std::endl;
        return;
    }

    // Listen for new connections
    if(listen(socketfd, 3) == -1)
    {
        std::cerr << "listen() failed: " << errno << std::endl;
        return;
    }

    // Accept connections 
    sockaddr_in client_socket;
    socklen_t client_size;
    int conn_socket = accept(socketfd, (sockaddr*) &client_socket, &client_size);
    if(conn_socket == -1)
    {
        std::cerr << "accept() failed: " << errno << std::endl;
        return;
    }

    // Send p and g for Diffie Hellman
    mpz_class p = 500, g = 500;
    char buf[8]; 

    if(send(conn_socket, &p, sizeof(p), 0) == -1)
    {
        std::cerr << "send() failed: " << errno << std::endl;
        return;
    }

    // wait for ok
    if(recv(conn_socket, buf, sizeof(buf), 0) == -1)
    {
        std::cerr << "recv() failed: " << errno << std::endl;
        return; 
    }

    // send g
    if(send(conn_socket, &g, sizeof(g), 0) == -1)
    {
        std::cerr << "send() failed: " << errno << std::endl;
        return;
    }

    // wait for ok

    // generate secret exp
}