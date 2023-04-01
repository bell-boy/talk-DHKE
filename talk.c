#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>

void server()
{
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("error code: %d\n", WSAGetLastError());
	}

	struct sockaddr_in addr;
	int addr_len = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8080);
	if(bind(sock, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("error code: %d\n", WSAGetLastError());
	}

	if(listen(sock, 1) == SOCKET_ERROR)
	{
		printf("error code: %d\n", WSAGetLastError());
	}
	int conn_socket;
	if((conn_socket = accept(sock, (sockaddr *) &addr, &addr_len)) == INVALID_SOCKET)
	{
		printf("error code: %d\n", WSAGetLastError());
	}
}

void client(u_long ip, u_short port)
{
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("error code: %d\n", WSAGetLastError());
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);
	int addr_size = sizeof(addr);
	if(connect(sock, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("error code: %d\n", WSAGetLastError());
	}
}

int main(int argc, char *argv[])
{
	WSADATA wsaDATA;
	WSAStartup(MAKEWORD(2, 2), &wsaDATA);
	if(argc > 1)
	{
		char ip_string[strlen(argv[1])];
		strcpy(ip_string, argv[1]);
		u_short port = atoi(argv[2]);	

		//check that all 4 octets exist
		size_t periods = 0;
		for(int i = 0; ip_string[i] != '\0'; i++)
		{
			if(ip_string[i] == '.')
				periods++;
		}
		if(periods != 3)
		{
			printf("invalid ip address\n");
			return 1;
		}

		//check that port number is valid
		if(port == 0)
		{
			printf("invalid port\n");
			return 1;
		}
		//convert octets to single unsigned long int
		u_long ip = 0;
		char octet[4] = {0, 0, 0, 0};
		int octet_index = 0;
		int octet_power = 3;
		for(int i = 0; ip_string[i] != '\0'; i++)
		{
			if(ip_string[i] == '.')
			{
				int addend = atoi(octet);
				for(int i = 0; i < octet_power; i++)
					addend *= 256;
				ip += addend;
				octet_index = 0;
				for(int i = 0; i < 4; i++)
					octet[i] = 0;
				octet_power--;
				continue;
			}
			octet[octet_index] = ip_string[i];	
			octet_index++;
			if(i == strlen(ip_string) - 1)
			{
				u_long addend = atoi(octet);
				for(int i = 0; i < octet_power; i++)
					addend *= 256;
				ip += addend;
				octet_index = 0;
				for(int i = 0; i < 4; i++)
					octet[i] = 0;
				octet_power--;
			}
		}

		client(ip, port);
	}
	else
		server();
	
	return 0;
}
