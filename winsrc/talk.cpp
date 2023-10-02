#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#define PRIME 71343919
#define PROOT 3

long long SECRET;
int running = 1;

// TODO: Research Faster method of integer exponetiation (square method?).
long long ipow(long long base, long long exp)
{
	long long res = 1;
	for(int i = 0; i < exp; i++)
		res *= base;
	return res;
}

// TODO: Move listen to main thread and talk to background thread, it allows for easier exit.

// This is a DWORD WINAPI becuase it allows the function to run as it's own thread.
DWORD WINAPI listen_for_messages(void* sock)
{
	char buf[256];
	while(running == 1)
	{
		if(recv((int)sock, buf, sizeof(buf), 0) == SOCKET_ERROR)
		{
			printf("listen error code: %d\n", WSAGetLastError());
			running = 0;
		}
		printf("them: %s\n", buf);
		if(strcmp(buf, "exit") == 0) 
			running = 0;
	}
	return 0;
}

// While this does NOT need to be a DWORD WINAPI since it's run in the main thread, I see no reason to change it.
DWORD WINAPI send_messages(void* sock)
{
	char buf[256];
	while(running == 1)
	{
		// TODO: change gets() to a SAFE function.
		gets(buf);
		if(send((int)sock, buf, sizeof(buf), 0) == SOCKET_ERROR)
		{
			printf("send error code: %d\n", WSAGetLastError());
			running = 0;
		}
		if(strcmp(buf, "exit") == 0) running = 0;
	}
	return 0;
}

// Handles server mode
void server(u_short port)
{
	int sock; // server socket
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("error code: %d\n", WSAGetLastError());
	}
	struct sockaddr_in addr; // server adress
	int addr_len = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if(bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("error code: %d\n", WSAGetLastError());
		return;
	}

	if(listen(sock, 1) == SOCKET_ERROR)
	{
		printf("error code: %d\n", WSAGetLastError());
		return;
	}

	// this just gets this host's ip address to display.
	char host[256];
	gethostname(host, sizeof(host));
	struct hostent* host_info = gethostbyname(host);	
	printf("server with ip %s is now listening on port %d\n", inet_ntoa(*((struct in_addr *)host_info->h_addr_list[0])), port);

	// conn_sock is the socket that refers to the client
	int conn_sock;
	if((conn_sock = accept(sock, (struct sockaddr *) &addr, &addr_len)) == INVALID_SOCKET)
	{
		printf("error code: %d\n", WSAGetLastError());
		return;
	}

	printf("connected to %s\n", inet_ntoa(addr.sin_addr));

	// Diffie-Hellman Key Exchange
	/*
	 * The main problem with this implementation is that the keys DHKE generates are too large.
	 * I've cut down the max_value of SECRET however, the keys just can't fit in a 64 bit integer.
	 * Now 128 bit integers might work, however since they're included as part of the compiler and NOT as part of the langauge standard, displaying them is a hassle.
	 * I think for now this is good enough as a proof of concept, since i'm not using the keys to encrypt anything, just to implement.
	 */
	char num_buf[sizeof(long long)];
	long long A = ipow(PROOT, SECRET) % PRIME; 
	long long B;
	memcpy(num_buf, &A, sizeof(long long));
	if(send((int)conn_sock, num_buf, sizeof(num_buf), 0) == SOCKET_ERROR)
	{
		printf("send error code: %d\n", WSAGetLastError());
		return;
	}
	printf("A (%lld) has been sent, Waiting for B\n", A);
	
	if(recv((int)conn_sock, num_buf, sizeof(num_buf), 0) == SOCKET_ERROR)
	{
		printf("listen error code: %d\n", WSAGetLastError());
		return;
	}
	memcpy(&B, num_buf, sizeof(long long));
	printf("B (%lld) has been received\n", B);
	
	printf("Diffie-Hellman Complete key is: %lld\n", ipow(B, SECRET) % PRIME);

	// This starts the program listening for any incomming messages on a seprate thread. 
	HANDLE listen_thread;	
	if((listen_thread = CreateThread(NULL, 0, listen_for_messages, (void*)conn_sock, 0, NULL)) == NULL)
	{
		printf("error code: %d\n", GetLastError());
		return;
	}
	// On this thread, we pause and wait for messages before sending them.
	send_messages((void*)conn_sock);
}

// handles the client mode
void client(u_long ip, u_short port)
{
	// this is the client socket
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("error code: %d\n", WSAGetLastError());
	}
	// client address info
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);
	int addr_size = sizeof(addr);

	if(connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("error code: %d\n", WSAGetLastError());
		return;
	}
	printf("connection successful\n");

	// This is the other side of the DHKE explained on line 94
	long long A, B = ipow(PROOT, SECRET) % PRIME;
	char num_buf[sizeof(long long)];
	if(recv((int)sock, num_buf, sizeof(num_buf), 0) == SOCKET_ERROR)
	{
		printf("listen error code: %d\n", WSAGetLastError());
		return;
	}
	memcpy(&A, num_buf, sizeof(num_buf));

	printf("A (%lld) received, sending B.\n", A); 

	memcpy(num_buf, &B, sizeof(long long));
	if(send((int)sock, num_buf, sizeof(num_buf), 0) == SOCKET_ERROR)
	{
		printf("send error code: %d\n", WSAGetLastError());
		return;
	}

	printf("B (%lld) has been sent.\n", B);
	printf("Diffie-Hellman Complete key is: %lld\n", ipow(A, SECRET) % PRIME);


	HANDLE listen_thread;	
	if((listen_thread = CreateThread(NULL, 0, listen_for_messages, (void*)sock, 0, NULL)) == NULL)
	{
		printf("error code: %d\n", GetLastError());
		return;
	}
	send_messages((void*)sock);
}

int main(int argc, char *argv[])
{
	// This seeds the random number generator. Technically the generator is psudeo-random which is not good enough in a real setting.
	// It suits the purposes of demonstration
	srand(time(NULL));
	SECRET = rand()%50 + 1;

	// These two lines are a windows thing, It initalizs the windows socket api.
	WSADATA wsaDATA;
	WSAStartup(MAKEWORD(2, 2), &wsaDATA);
	
	// this handles program inputs
	// only providing a port will start a SERVER
	// providing an ip adrress and port will start a CLIENT trying to connect to the SERVER at the given ip adress.
	if(argc > 2)
	{
		// TODO: SIMPLFIY THE READING OF IP ADDRESSES
		// there is a built in function for this

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
