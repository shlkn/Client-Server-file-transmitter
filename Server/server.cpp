#include "server.h"

void printBytes(std::vector<uint8_t> bytes);
std::vector<uint8_t> int32ToBytes(uint32_t paramInt);

int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        std::cout << "Incorrect starting parameters. Must be: sourceIP destIP sourcePort\n";
        return -1;
    }

    // Network configuratuion
    NetworkManager networkManager(argv[1], argv[2], atoi(argv[3]), 0);
    if(!networkManager.bindSocket())
        return -1;

    Server server(networkManager);
    server.mainLoop();
}


Server::Server(NetworkManager &_networkManager)
{
    networkManager = &_networkManager;
}

void Server::mainLoop()
{
    while(true)
    {
        int apiRetVal = 0, srcPort;
        auto packet = networkManager->receivePacket(apiRetVal, srcPort);

        //parsing service part
        std::vector<uint8_t> seviceBytes(SERVICE_INFORMATION_SIZE);
        copy_n(packet.begin(), seviceBytes.size(), seviceBytes.begin());
        auto serviceDataDecoded = proto.decodeServiceInformation(seviceBytes);

        //parsing file segment
        auto fileSegment = std::make_pair(serviceDataDecoded.seq_number, FileFragment());
        copy_n(packet.begin() + seviceBytes.size(), fileSegment.second.fileData.size(), fileSegment.second.fileData.begin());
        fileSegment.second.actualSizeOfFileData = apiRetVal - SERVICE_INFORMATION_SIZE;

        // making ACK packet
        serviceDataDecoded.type = 1; 
        auto ACKBytes = proto.encodeServiceInformation(serviceDataDecoded);

        // saving file fragment data
        if(auto fileToReceive = filesToReceive.find(serviceDataDecoded.id); fileToReceive != filesToReceive.end())
        {
            fileToReceive->second.fileFragments.insert(std::move(fileSegment));

            //check for last packet
            if(fileToReceive->second.fileFragments.size() == fileToReceive->second.seq_total)
            {   
                fileToReceive->second.calculateHashSumm();
                auto hashInBytes = int32ToBytes(fileToReceive->second.hashSumm);
                for(auto byte : hashInBytes)
                    ACKBytes.push_back(byte);
                std::cout << "file received, hash = " << std::hex << fileToReceive->second.hashSumm << std::endl;
            } 
        }
        else
        {
            auto fileToReceiveNew = std::make_pair(serviceDataDecoded.id, FileToReceive());
            fileToReceiveNew.second.fileFragments.insert(std::move(fileSegment));
            fileToReceiveNew.second.seq_total = serviceDataDecoded.seq_total;
            if(fileToReceiveNew.second.fileFragments.size() == fileToReceiveNew.second.seq_total)
            {
                fileToReceiveNew.second.calculateHashSumm();
                auto hashInBytes = int32ToBytes(fileToReceiveNew.second.hashSumm);
                for(auto byte : hashInBytes)
                    ACKBytes.push_back(byte);
                std::cout << "file received, hash = " << std::hex << fileToReceive->second.hashSumm << std::endl;
            }
            filesToReceive.insert(std::move(fileToReceiveNew));        
        }

        //sending ACK packet
        networkManager->sendPacket(ACKBytes, srcPort);
    }
}