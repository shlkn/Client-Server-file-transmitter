#include "networkmanager.h"

NetworkManager::NetworkManager(std::string _sourceIP, std::string _destIP, int _sourcePort, int _destPort)
{
    sourcePort = _sourcePort;
    destPort = _destPort;
    sourceIP = _sourceIP;
    destIP = _destIP;

    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr(destIP.c_str());
    destAddr.sin_port = htons(destPort);  
    lenOfDestAddr = sizeof(destAddr);
}

bool NetworkManager::bindSocket()
{
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    { 
        std::cout << "Can`t create socket\n"; 
        return false;
    } 

    struct sockaddr_in clientAddr;

    clientAddr.sin_family    = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(sourceIP.c_str()); 
    clientAddr.sin_port = htons(sourcePort); 


    if (bind(sockfd, (const struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) 
    { 
        std::cout << "Can`t bind socket\n"; 
        return false;
    }

    return true; 
}

NetworkManager::~NetworkManager()
{
    close(sockfd);
}

void NetworkManager::sendPacket(std::vector<uint8_t> bytesToSend)
{
    sendto(sockfd, static_cast<const void*>(bytesToSend.data()), bytesToSend.size(), 0, (const struct sockaddr *) &destAddr, lenOfDestAddr);
}

void NetworkManager::sendPacket(std::vector<uint8_t> bytesToSend, int destPort)
{
    destAddr.sin_port = htons(destPort); 
    sendto(sockfd, static_cast<const void*>(bytesToSend.data()), bytesToSend.size(), 0, (const struct sockaddr *) &destAddr, lenOfDestAddr);
}

std::vector<uint8_t> NetworkManager::receivePacket(int &apiRetVal)
{
    std::vector<uint8_t> recvBuff(MAX_UDP_PAYLOAD);
    
    apiRetVal = recv(sockfd, (recvBuff.data()), MAX_UDP_PAYLOAD, 0);

    return recvBuff;
}

std::vector<uint8_t> NetworkManager::receivePacket(int &apiRetVal, int &portFrom)
{
    std::vector<uint8_t> recvBuff(MAX_UDP_PAYLOAD);
    struct sockaddr_in from;
    socklen_t len = sizeof(from);

    apiRetVal = recvfrom(sockfd, (recvBuff.data()), MAX_UDP_PAYLOAD, 0, ( struct sockaddr *) &from, &len);
    portFrom = htons(from.sin_port);

    return recvBuff;
}


void NetworkManager::setSockTimeout(int timeoutSecs)
{
    struct timeval tv;
    tv.tv_sec = timeoutSecs;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
}

uint32_t bytesToInt32(std::array<uint8_t, 4> arrayOfByte)
{
    int32_t paramInt = 0;
    paramInt |= (int32_t) arrayOfByte[0] << 24;
    paramInt |= (int32_t) arrayOfByte[1] << 16;
    paramInt |= (int32_t) arrayOfByte[2] <<  8;
    paramInt |= (int32_t) arrayOfByte[3];
    return paramInt;
}

uint64_t bytesToInt64(std::array<uint8_t, 8> arrayOfByte)
{
    int64_t paramInt = 0;
    paramInt |= (int64_t) arrayOfByte[0] << 56;
    paramInt |= (int64_t) arrayOfByte[1] << 48;
    paramInt |= (int64_t) arrayOfByte[2] << 40;
    paramInt |= (int64_t) arrayOfByte[3] << 32;
    paramInt |= (int64_t) arrayOfByte[4] << 24;
    paramInt |= (int64_t) arrayOfByte[5] << 16;
    paramInt |= (int64_t) arrayOfByte[6] <<  8;
    paramInt |= (int64_t) arrayOfByte[7];
    return paramInt;
}

std::vector<uint8_t> int32ToBytes(uint32_t paramInt)
{
     std::vector<unsigned char> arrayOfByte(4);
     for (int i = 0; i < 4; i++)
         arrayOfByte[3 - i] = (paramInt >> (i * 8));
     return arrayOfByte;
}

std::vector<uint8_t> int64ToBytes(uint64_t paramInt)
{
     std::vector<unsigned char> arrayOfByte(8);
     for (int i = 0; i < 8; i++)
         arrayOfByte[7 - i] = (paramInt >> (i * 8));
     return arrayOfByte;
}

 std::vector<uint8_t> ProtoManager::encodeServiceInformation(ServiceInformation dataToencode)
{
    auto vectorBytes = int32ToBytes(dataToencode.seq_number);
    
    for(auto byte : int32ToBytes(dataToencode.seq_total))
        vectorBytes.push_back(byte);
    
    vectorBytes.push_back(dataToencode.type);

    for(auto byte : int64ToBytes(dataToencode.id))
        vectorBytes.push_back(byte);

    return vectorBytes;
}

ProtoManager::ServiceInformation ProtoManager::decodeServiceInformation(std::vector<uint8_t> serviceInformationBytes)
{
    ProtoManager::ServiceInformation outData;

    std::array<uint8_t, 4> int32ParamBytes;
    std::array<uint8_t, 8> int64ParamBytes;
    auto startIter = serviceInformationBytes.begin();

    std::copy_n(startIter, int32ParamBytes.size(), int32ParamBytes.begin());
    outData.seq_number = bytesToInt32(int32ParamBytes);
    startIter += int32ParamBytes.size();

    std::copy_n(startIter, int32ParamBytes.size(), int32ParamBytes.begin());
    outData.seq_total = bytesToInt32(int32ParamBytes);
    startIter += int32ParamBytes.size();

    outData.type = *startIter;
    startIter++;

    std::copy_n(startIter, int64ParamBytes.size(), int64ParamBytes.begin());
    outData.id = bytesToInt64(int64ParamBytes);

    return outData;
}


uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len)
{
    int k;
    crc = ~crc;
    while (len--) 
    {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
    }

    return ~crc;
}