#ifndef NETWORK_MANAGER
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unistd.h>

#define SERVICE_INFORMATION_SIZE 17
#define MAX_UDP_PAYLOAD 1472
#define MAX_DATA_SIZE MAX_UDP_PAYLOAD - SERVICE_INFORMATION_SIZE

struct FileFragment
{
    int actualSizeOfFileData = 0;
    std::array<uint8_t, MAX_DATA_SIZE> fileData;
};

class NetworkManager
{
public:
    NetworkManager(std::string _sourceIP, std::string _destIP, int _sourcePort, int _destPort);
    ~NetworkManager();
    bool bindSocket();
    void sendPacket(std::vector<uint8_t> bytesToSend);
    void sendPacket(std::vector<uint8_t> bytesToSend, int destPort);
    std::vector<uint8_t> receivePacket(int &apiRetVal);
    std::vector<uint8_t> receivePacket(int &apiRetVal, int &portFrom);
    void setSockTimeout(int timeoutSecs);

private:
    int sourcePort, destPort;
    std::string sourceIP, destIP;
    struct sockaddr_in destAddr;
    int lenOfDestAddr = 0;
    int sockfd; 
};

class ProtoManager
{
public:
    struct ServiceInformation
    {
        uint32_t seq_number = 0; // номер пакета
        uint32_t seq_total = 0; // количество пакетов с данными
        uint8_t type = 0; // тип пакета: 0 == ACK, 1 == PUT
        uint64_t id = 0; // 8 байт - идентификатор, отличающий один файл от другого

        void print()
        {
            std::cout << "seq_number - " << std::dec << seq_number << std::endl;
            std::cout << "seq_total - " << seq_total << std::endl;
            std::cout << "type - " << (int)type << std::endl;
            std::cout << "id - " << id << std::endl;
        }
    };

    std::vector<uint8_t> encodeServiceInformation(ServiceInformation dataToencode);
    ServiceInformation decodeServiceInformation(std::vector<uint8_t> serviceInformationBytes);

};
#endif