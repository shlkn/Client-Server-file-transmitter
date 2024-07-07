#include "../networkmanager.h"

uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);

struct FileToReceive
{
    std::unordered_map<uint32_t, struct FileFragment> fileFragments;
    uint32_t hashSumm = 0;
    uint32_t seq_total = 0;

    void calculateHashSumm()
    {
        uint32_t hash = 0;
        for(int i = 1; i != fileFragments.size() + 1; i++)
            if(auto fileFragment = fileFragments.find(i); fileFragment != fileFragments.end())
                hash = crc32c(hash, fileFragment->second.fileData.begin(), fileFragment->second.actualSizeOfFileData);
        hashSumm = hash;
    }
};

class Server
{
public:
    Server(NetworkManager &_networkManager);
    void mainLoop();
private:
    std::unordered_map<uint32_t, struct FileToReceive> filesToReceive; // key - file id;
    NetworkManager *networkManager;
    ProtoManager proto;
};