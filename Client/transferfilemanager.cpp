#include "transferfilemanager.h"

uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);
uint32_t bytesToInt32(std::array<uint8_t, 4> arrayOfByte);

std::vector<uint8_t> FileToTransfer::getFragmentToSend()
{
    ProtoManager::ServiceInformation serviceData;
    serviceData.seq_number = currentFileFragment->first;
    serviceData.seq_total = countOfFragments;
    serviceData.type = 0;
    serviceData.id = fileID;
    auto bytesToSend = proto.encodeServiceInformation(serviceData);
    bytesToSend.reserve(MAX_UDP_PAYLOAD);

    std::copy_n(currentFileFragment->second.fileData.begin(), currentFileFragment->second.actualSizeOfFileData, std::back_inserter(bytesToSend));

    return bytesToSend;
}


FileToTransfer::FileToTransfer(std::unordered_map<uint32_t, struct FileFragment> _fileFragments, NetworkManager &_networkManager)
{
    fileFragments = std::move(_fileFragments);
    countOfFragments = fileFragments.size();
    currentFileFragment = fileFragments.begin();

    // calculating hashSumm
    for(int i = 1; i != fileFragments.size() + 1; i++)
    {   
        if(auto fileFragment = fileFragments.find(i); fileFragment != fileFragments.end())
        {
            fileHashSumm = crc32c(fileHashSumm, fileFragment->second.fileData.begin(), fileFragment->second.actualSizeOfFileData);
        }
    }
    fileID = fileHashSumm; // using hashSumm like file id

    networkManager = &_networkManager;
}

bool FileToTransfer::incrementToNextfragment()
{
    currentFileFragment++;

    std::this_thread::sleep_for(std::chrono::nanoseconds(DELAY_BETWEEN_PACKETS)); 
    if(currentFileFragment != fileFragments.end())
        return true;

    return false;
}

void FileToTransfer::transferFileStart()
{
    do
    {
        auto bytesToSend = getFragmentToSend();
        networkManager->sendPacket(bytesToSend);

        while(true)
        {
            // waiting for ACK
            int apiRetVal = 0;
            auto packet = networkManager->receivePacket(apiRetVal);
            if(apiRetVal == -1)
            {
                std::cout << "retry sending fragment" << std::endl;
                networkManager->sendPacket(bytesToSend);
            }
            else
            {
                std::vector<uint8_t> seviceBytes(SERVICE_INFORMATION_SIZE);
                copy_n(packet.begin(), seviceBytes.size(), seviceBytes.begin());
                auto serviceDataDecoded = proto.decodeServiceInformation(seviceBytes);

                if(apiRetVal == SERVICE_INFORMATION_SIZE + sizeof(fileHashSumm))
                {
                    std::array<uint8_t, sizeof(fileHashSumm)> hashBytes;
                    copy_n(packet.begin() + SERVICE_INFORMATION_SIZE, hashBytes.size(), hashBytes.begin());
                    auto hashFromServer = bytesToInt32(hashBytes);

                    if(hashFromServer == fileHashSumm)
                        std::cout << "file transfer complete. CRC32C codes equal\n";
                    else
                        std::cout << "file transfer unsuccessful. CRC32C codes not equal\n";                    
                }

                if(serviceDataDecoded.seq_number == currentFileFragment->first)
                    break;
            }
        }
    }while(incrementToNextfragment());
} 