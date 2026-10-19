
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

std::mutex clientsMutex;
std::mutex usersMutex;

std::unordered_map<std::string, SOCKET> clients;
std::unordered_map<std::string, std::string> users;

void HandleClient(SOCKET client)
{
    char buffer[4096];

    std::string userId;

    while (true)
    {
        int received = recv(client, buffer, sizeof(buffer) - 1, 0);

        if (received <= 0)
            break;

        buffer[received] = '\0';
        std::string message(buffer);

        // Protocol: REGISTER|userId|password
        // Protocol: LOGIN|userId|password
        // Protocol: MSG|message text
        // Protocol: LOGOUT

        if (message.rfind("REGISTER|", 0) == 0 ||
            message.rfind("LOGIN|", 0) == 0)
        {
            bool registration = message.rfind("REGISTER|", 0) == 0;

            std::stringstream ss(message);
            std::string command, id, password;

            std::getline(ss, command, '|');
            std::getline(ss, id, '|');
            std::getline(ss, password);

            std::lock_guard<std::mutex> lock(usersMutex);

            if (id.empty() || password.empty())
            {
                send(client, "ERROR|Missing credentials\n", 26, 0);
            }
            else if (registration)
            {
                if (users.count(id))
                {
                    std::string response = "ERROR|User already exists\n";
                    send(client, response.c_str(),
                        static_cast<int>(response.size()), 0);
                }
                else
                {
                    users[id] = password;
                    userId = id;

                    std::string response = "OK|REGISTERED|" + id + "\n";
                    send(client, response.c_str(),
                        static_cast<int>(response.size()), 0);
                }
            }
            else
            {
                auto it = users.find(id);

                if (it != users.end() && it->second == password)
                {
                    userId = id;

                    std::string response = "OK|LOGIN|" + id + "\n";
                    send(client, response.c_str(),
                        static_cast<int>(response.size()), 0);
                }
                else
                {
                    std::string response = "ERROR|Invalid credentials\n";
                    send(client, response.c_str(),
                        static_cast<int>(response.size()), 0);
                }
            }
        }
        else if (message.rfind("MSG|", 0) == 0)
        {
            if (userId.empty())
            {
                std::string response = "ERROR|Login required\n";
                send(client, response.c_str(),
                    static_cast<int>(response.size()), 0);
                continue;
            }

            std::string chat = message.substr(4);
            std::cout << "[" << userId << "] " << chat;

            if (chat.empty() || chat.back() != '\n')
                std::cout << '\n';

            std::string response = "OK|MESSAGE_RECEIVED\n";
            send(client, response.c_str(),
                static_cast<int>(response.size()), 0);
        }
        else if (message.rfind("LOGOUT", 0) == 0)
        {
            userId.clear();
            std::string response = "OK|LOGOUT\n";
            send(client, response.c_str(),
                static_cast<int>(response.size()), 0);
        }
        else
        {
            std::string response = "ERROR|Unknown command\n";
            send(client, response.c_str(),
                static_cast<int>(response.size()), 0);
        }
    }

    if (!userId.empty())
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(userId);
    }

    closesocket(client);
}

int main()
{
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "Winsock initialization failed.\n";
        return 1;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(5000);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server, reinterpret_cast<sockaddr*>(&address),
        sizeof(address)) == SOCKET_ERROR ||
        listen(server, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Could not bind/listen on TCP port 5000. "
            "Check whether another server is using it.\n";

        closesocket(server);
        WSACleanup();
        return 1;
    }

    std::cout << "====================================\n";
    std::cout << "       NEXORA TCP SERVER\n";
    std::cout << "====================================\n";
    std::cout << "Listening on port 5000...\n";
    std::cout << "Waiting for clients.\n";

    while (true)
    {
        SOCKET client = accept(server, nullptr, nullptr);

        if (client == INVALID_SOCKET)
        {
            std::cerr << "Client accept failed.\n";
            continue;
        }

        std::cout << "Client connected.\n";

        std::thread(HandleClient, client).detach();
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
