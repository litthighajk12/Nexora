
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <atomic>
#include <algorithm>
#include <unordered_map>
#include <chrono>

#include "MessageFraming.h"

#pragma comment(lib, "Ws2_32.lib")

struct Client
{
    SOCKET socket = INVALID_SOCKET;
    int clientId = 0;

    std::mutex sendMutex;
    std::string receiveBuffer;

    std::atomic<bool> connected{ true };
};

struct IoOperation
{
    OVERLAPPED overlapped{};
    WSABUF buffer{};
    char data[1024]{};
    DWORD flags = 0;
    std::shared_ptr<Client> client;
};

HANDLE g_iocp = nullptr;

std::mutex g_clientsMutex;
std::vector<std::shared_ptr<Client>> g_clients;
std::atomic<int> g_nextClientId{ 1 };
SOCKET g_udpSocket = INVALID_SOCKET;

std::mutex g_presenceMutex;

using Clock = std::chrono::steady_clock;

struct PresenceInfo
{
    Clock::time_point lastHeartbeat;
    bool active = true;
};

std::unordered_map<std::string, PresenceInfo> g_lastHeartbeat;


// Send all bytes, accounting for partial sends.
bool SendAll(SOCKET socket, const std::string& data)
{
    size_t offset = 0;

    while (offset < data.size())
    {
        int sent = send(
            socket,
            data.data() + offset,
            static_cast<int>(data.size() - offset),
            0
        );

        if (sent == SOCKET_ERROR || sent == 0)
            return false;

        offset += static_cast<size_t>(sent);
    }

    return true;
}


// Broadcast one framed message to all connected clients.
void Broadcast(const std::string& message)
{
    const std::string frame = EncodeMessage(message) + '\n';

    std::vector<std::shared_ptr<Client>> clients;

    {
        std::lock_guard<std::mutex> lock(g_clientsMutex);
        clients = g_clients;
    }

    for (const auto& client : clients)
    {
        if (!client->connected)
            continue;

        bool sentSuccessfully = false;

        {
            std::lock_guard<std::mutex> sendLock(
                client->sendMutex
            );

            if (client->connected)
            {
                sentSuccessfully = SendAll(
                    client->socket,
                    frame
                );
            }
        }

        if (!sentSuccessfully)
        {
            std::cout << "Send failed for a client.\n";
        }
    }
}
bool SendToClient(int recipientId, const std::string& message)
{
    std::shared_ptr<Client> recipient;

    {
        std::lock_guard<std::mutex> lock(g_clientsMutex);

        for (const auto& client : g_clients)
        {
            if (client->clientId == recipientId &&
                client->connected)
            {
                recipient = client;
                break;
            }
        }
    }

    if (!recipient)
    {
        std::cout << "Recipient " << recipientId
            << " is not connected.\n";
        return false;
    }

    std::string frame = EncodeMessage(message) + '\n';

    std::lock_guard<std::mutex> sendLock(recipient->sendMutex);

    if (!recipient->connected)
        return false;

    if (!SendAll(recipient->socket, frame))
    {
        std::cout << "Failed to send to client "
            << recipientId << '\n';
        return false;
    }

    return true;
}
void ProcessMessage(
    const std::shared_ptr<Client>& sender,
    const std::string& message)
{
    // Broadcast format: B|message
    if (message.rfind("B|", 0) == 0)
    {
        std::string text = message.substr(2);

        std::cout << "Client " << sender->clientId
            << " broadcasting: " << text << '\n';

        Broadcast(
            "Client " + std::to_string(sender->clientId)
            + ": " + text
        );

        return;
    }

    // Private format: P|recipientId|message
    if (message.rfind("P|", 0) == 0)
    {
        size_t separator = message.find('|', 2);

        if (separator == std::string::npos)
        {
            std::cout << "Invalid private message format.\n";
            return;
        }

        int recipientId = 0;

        try
        {
            size_t parsedLength = 0;

            recipientId = std::stoi(
                message.substr(2, separator - 2),
                &parsedLength
            );

            if (parsedLength != separator - 2 ||
                recipientId <= 0)
            {
                std::cout << "Invalid recipient ID.\n";
                return;
            }
        }
        catch (...)
        {
            std::cout << "Invalid recipient ID.\n";
            return;
        }

        std::string text = message.substr(separator + 1);

        std::string outgoing =
            "Private from Client " +
            std::to_string(sender->clientId) + ": " + text;

        std::cout << "Private message: Client "
            << sender->clientId << " -> Client "
            << recipientId << '\n';

        if (!SendToClient(recipientId, outgoing))
        {
            std::cout << "Private message delivery failed.\n";
        }

        return;
    }

    std::cout << "Unknown message type from Client "
        << sender->clientId << '\n';
}
bool PostReceive(
    const std::shared_ptr<Client>& client,
    IoOperation* op)
{
    op->client = client;
    op->buffer.buf = op->data;
    op->buffer.len = sizeof(op->data);
    op->flags = 0;

    DWORD received = 0;

    int result = WSARecv(
        client->socket,
        &op->buffer,
        1,
        &received,
        &op->flags,
        &op->overlapped,
        nullptr
    );

    return result == 0 ||
        WSAGetLastError() == WSA_IO_PENDING;
}


void RemoveClient(const std::shared_ptr<Client>& client)
{
    bool expected = true;

    if (!client->connected.compare_exchange_strong(
        expected, false))
    {
        return;
    }

    // Close the socket to terminate further communication.
    closesocket(client->socket);
    client->socket = INVALID_SOCKET;

    {
        std::lock_guard<std::mutex> lock(g_clientsMutex);

        g_clients.erase(
            std::remove(
                g_clients.begin(),
                g_clients.end(),
                client
            ),
            g_clients.end()
        );
    }

    std::cout << "A client disconnected.\n";
}


void WorkerThread()
{
    while (true)
    {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        OVERLAPPED* overlapped = nullptr;

        BOOL success = GetQueuedCompletionStatus(
            g_iocp,
            &bytesTransferred,
            &completionKey,
            &overlapped,
            INFINITE
        );

        if (overlapped == nullptr)
            continue;

        auto* op = reinterpret_cast<IoOperation*>(overlapped);
        auto client = op->client;

        if (!success || bytesTransferred == 0)
        {
            RemoveClient(client);
            delete op;
            continue;
        }

        if (!client->connected)
        {
            delete op;
            continue;
        }

        // Append received bytes. One receive may contain
        // a partial frame or several complete frames.
        client->receiveBuffer.append(
            op->data,
            bytesTransferred
        );

        std::vector<std::string> messages;

        if (!ExtractMessages(
            client->receiveBuffer,
            messages))
        {
            std::cout << "Invalid or oversized TCP frame.\n";

            RemoveClient(client);
            delete op;
            continue;
        }

        for (const auto& message : messages)
        {
            ProcessMessage(client, message);
        }

        // Prepare the operation for another receive.
        ZeroMemory(&op->overlapped, sizeof(op->overlapped));

        if (!client->connected ||
            !PostReceive(client, op))
        {
            RemoveClient(client);
            delete op;
        }
    }
}


void UdpHeartbeatReceiver()
{
    char buffer[256];

    while (true)
    {
        sockaddr_in sender{};
        int senderLength = sizeof(sender);

        int received = recvfrom(
            g_udpSocket,
            buffer,
            sizeof(buffer),
            0,
            reinterpret_cast<sockaddr*>(&sender),
            &senderLength
        );

        if (received == SOCKET_ERROR)
        {
            std::cout << "UDP receive failed: "
                << WSAGetLastError() << '\n';
            return;
        }

        char ipAddress[INET_ADDRSTRLEN]{};

        inet_ntop(
            AF_INET,
            &sender.sin_addr,
            ipAddress,
            sizeof(ipAddress)
        );

        std::string clientId =
            std::string(ipAddress) + ":" +
            std::to_string(ntohs(sender.sin_port));

        {
            std::lock_guard<std::mutex> lock(g_presenceMutex);

            auto& presence = g_lastHeartbeat[clientId];
            presence.lastHeartbeat = Clock::now();

            if (!presence.active)
            {
                presence.active = true;

                std::cout << "[UDP] Client active again: "
                    << clientId << '\n';
            }
        }

        // Suppress routine heartbeat logs to keep output readable.
    }
}


void HeartbeatTimeoutChecker()
{
    while (true)
    {
        std::this_thread::sleep_for(
            std::chrono::seconds(1)
        );

        auto now = Clock::now();

        std::lock_guard<std::mutex> lock(g_presenceMutex);

        for (auto& entry : g_lastHeartbeat)
        {
            auto elapsed =
                std::chrono::duration_cast<std::chrono::seconds>(
                    now - entry.second.lastHeartbeat
                ).count();

            if (entry.second.active && elapsed > 15)
            {
                entry.second.active = false;

                std::cout << "[UDP] Client inactive: "
                    << entry.first
                    << " (last heartbeat "
                    << elapsed << " seconds ago)\n";
            }
        }
    }
}


int main()
{
    WSADATA wsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "Winsock initialization failed.\n";
        return 1;
    }

    SOCKET listenSocket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (listenSocket == INVALID_SOCKET)
    {
        std::cout << "TCP socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(5000);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(
        listenSocket,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)) == SOCKET_ERROR ||
        listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cout << "Bind/listen failed: "
            << WSAGetLastError() << '\n';

        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    g_iocp = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        nullptr,
        0,
        0
    );

    if (!g_iocp)
    {
        std::cout << "IOCP creation failed.\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    unsigned int workerCount =
        std::thread::hardware_concurrency();

    if (workerCount == 0)
        workerCount = 2;

    std::vector<std::thread> workers;

    for (unsigned int i = 0; i < workerCount; ++i)
        workers.emplace_back(WorkerThread);

    std::cout << "IOCP chat server listening on port 5000...\n";

    // UDP heartbeat socket
    g_udpSocket = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    if (g_udpSocket == INVALID_SOCKET)
    {
        std::cout << "UDP socket creation failed: "
            << WSAGetLastError() << '\n';
        return 1;
    }

    sockaddr_in udpAddress{};
    udpAddress.sin_family = AF_INET;
    udpAddress.sin_port = htons(5001);
    udpAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(
        g_udpSocket,
        reinterpret_cast<sockaddr*>(&udpAddress),
        sizeof(udpAddress)) == SOCKET_ERROR)
    {
        std::cout << "UDP bind failed: "
            << WSAGetLastError() << '\n';

        closesocket(g_udpSocket);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::thread(UdpHeartbeatReceiver).detach();
    std::thread(HeartbeatTimeoutChecker).detach();

    std::cout << "UDP presence listening on port 5001...\n";

    // Accept clients
    while (true)
    {
        SOCKET clientSocket = accept(
            listenSocket,
            nullptr,
            nullptr
        );

        if (clientSocket == INVALID_SOCKET)
            continue;

        auto client = std::make_shared<Client>();

        client->socket = clientSocket;
        client->clientId = g_nextClientId++;

        std::string welcome =
            "WELCOME|" + std::to_string(client->clientId);

        if (!SendAll(client->socket, EncodeMessage(welcome) + '\n'))
        {
            closesocket(clientSocket);
            continue;
        }
        if (!CreateIoCompletionPort(
            reinterpret_cast<HANDLE>(clientSocket),
            g_iocp,
            0,
            0))
        {
            closesocket(clientSocket);
            continue;
        }

        size_t clientCount = 0;

        {
            std::lock_guard<std::mutex> lock(g_clientsMutex);

            g_clients.push_back(client);
            clientCount = g_clients.size();
        }

        std::cout << "Client " << client->clientId<< " connected. Total: " << clientCount << '\n';

        auto* op = new IoOperation{};

        if (!PostReceive(client, op))
        {
            RemoveClient(client);
            delete op;
        }
    }

    return 0;
}
