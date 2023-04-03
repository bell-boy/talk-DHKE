#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

DWORD WINAPI listen_for_messages(void* sock)
{
	char buf[256];
	while(1)
	{
		if(recv((int)sock, buf, sizeof(buf), 0) == SOCKET_ERROR)
		{
			printf("listen error code: %d\n", WSAGetLastError());
			break;
		}
		printf("them: %s\n", buf);
	}
	return 0;
}

DWORD WINAPI send_messages(void* sock)
{
	char buf[256];
	while(1)
	{
		gets(buf);
		if(send((int)sock, buf, sizeof(buf), 0) == SOCKET_ERROR)
		{
			printf("send error code: %d\n", WSAGetLastError());
			break;
		}
	}
	return 0;
}

void server(u_short port)
{
	int public_key = rand();
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("error code: %d\n", WSAGetLastError());
	}
	struct sockaddr_in addr;
	int addr_len = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if(bind(sock, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("error code: %d\n", WSAGetLastError());
		return;
	}

	if(listen(sock, 1) == SOCKET_ERROR)
	{
		printf("error code: %d\n", WSAGetLastError());
		return;
	}

	char host[256];
	gethostname(host, sizeof(host));
	struct hostent* host_info = gethostbyname(host);	
	printf("server with ip %s is now listening on port %d\n", inet_ntoa(*((struct in_addr *)host_info->h_addr_list[0])), port);
	int conn_sock;
	if((conn_sock = accept(sock, (sockaddr *) &addr, &addr_len)) == INVALID_SOCKET)
	{
		printf("error code: %d\n", WSAGetLastError());
		return;
	}

	printf("connected to %s\n", inet_ntoa(addr.sin_addr));
	char num_buf[sizeof(int)];
	memcpy(num_buf, &public_key, sizeof(int));
	if(send((int)conn_sock, num_buf, sizeof(num_buf), 0) == SOCKET_ERROR)
	{
		printf("send error code: %d\n", WSAGetLastError());
		return;
	}
	printf("public key %d sent\n", public_key);

	HANDLE listen_thread, talk_thread;	
	if((listen_thread = CreateThread(NULL, 0, listen_for_messages, (void*)conn_sock, 0, NULL)) == NULL)
	{
		printf("error code: %d\n", GetLastError());
		return;
	}
	send_messages((void*)conn_sock);
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
	int conn_sock;
	if((conn_sock = connect(sock, (sockaddr *) &addr, sizeof(addr))) == SOCKET_ERROR)
	{
		printf("error code: %d\n", WSAGetLastError());
		return;
	}
	printf("connection successful\n");
	char num_buf[sizeof(int)];
	if(recv((int)sock, num_buf, sizeof(num_buf), 0) == SOCKET_ERROR)
	{
		printf("listen error code: %d\n", WSAGetLastError());
		return;
	}
	int public_key;
	memcpy(&public_key, num_buf, sizeof(num_buf));
	printf("public key %d recived\n", public_key);
	HANDLE listen_thread, talk_thread;	
	if((listen_thread = CreateThread(NULL, 0, listen_for_messages, (void*)sock, 0, NULL)) == NULL)
	{
		printf("error code: %d\n", GetLastError());
		return;
	}
	send_messages((void*)sock);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	WSADATA wsaDATA;
	WSAStartup(MAKEWORD(2, 2), &wsaDATA);
	if(argc > 2)
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
	{
		u_short port = atoi(argv[1]);
		if(argc == 1 || port == 0)
		{
			printf("invalid port\n");
			return 1;
		}
		server(port);
	}
	
	return 0;
}
