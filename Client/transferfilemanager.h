#include "../networkmanager.h"
#include <thread>

#define DELAY_BETWEEN_PACKETS 1 

class FileToTransfer
{
public:
    FileToTransfer(std::unordered_map<uint32_t, struct FileFragment> _fileFragments, NetworkManager &_networkManager);
    std::vector<uint8_t> getFragmentToSend();
    void transferFileStart();


private:
    bool incrementToNextfragment();

    std::unordered_map<uint32_t, struct FileFragment> fileFragments;
    int countOfFragments = 0;
    uint64_t fileID = 0;
    uint32_t fileHashSumm = 0;
    std::unordered_map<uint32_t, struct FileFragment>::iterator currentFileFragment;
    ProtoManager proto;
    NetworkManager *networkManager;
};