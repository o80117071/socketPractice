#include <WinSock2.h>
#include <iostream>
#include <thread>

#pragma comment(lib,"Ws2_32.lib")
using namespace std;

class AutoWSACleanup final
{
public:
	AutoWSACleanup() = default;
	~AutoWSACleanup()
	{
		WSACleanup();
	}
};

class AutoSocketCleanup final
{
public:
	AutoSocketCleanup(SOCKET& socket) : m_socket(socket) {}
	~AutoSocketCleanup()
	{
		closesocket(m_socket);
	}
private:
	SOCKET m_socket;
};

int main()
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
		return 0;
	AutoWSACleanup auto_wsa_cleanup;

	SOCKET listen_soceket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_soceket == INVALID_SOCKET)
		return 0;
	AutoSocketCleanup auto_close_listen_socket(listen_soceket);

	sockaddr_in socket_address;
	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.S_un.S_addr = INADDR_ANY;
	socket_address.sin_port = htons(2500);
	if (bind(listen_soceket, reinterpret_cast<sockaddr*>(&socket_address), sizeof(socket_address)) == SOCKET_ERROR)
		return 0;

	if (listen(listen_soceket, SOMAXCONN) == SOCKET_ERROR)
	{
		return 0;
	}

	while (true)
	{
		cout << "Waiting client connect..." << endl;
		SOCKET client_socket = accept(listen_soceket, nullptr, nullptr);
		if (client_socket == INVALID_SOCKET)
			continue;

		std::thread t([&client_socket]()
			{
				AutoSocketCleanup auto_close_client_socket(client_socket);

				const char message[] = "Hello client";
				int send_len = send(client_socket, message, strlen(message), 0);
				if (send_len == SOCKET_ERROR)
					return 0;

				int recv_size = 0;
				do
				{
					char recv_buffer[512] = {};
					recv_size = recv(client_socket, recv_buffer, sizeof(recv_buffer) - 1, 0);
					if (recv_size > 0)
					{						
						// handle data
						cout << recv_buffer;
					}
					else if (recv_size == 0)
					{
						// client close socket
					}
					else if (recv_size == SOCKET_ERROR)
					{
						// error
						return 0;
					}
				} while (recv_size != 0);
			});

		t.detach();
	}
	return 0;
}