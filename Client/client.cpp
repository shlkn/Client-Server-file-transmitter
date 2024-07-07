#include "filereader.h"

#define SOCKET_WAIT_TIME_IN_SEC 5

int main(int argc, char *argv[])
{
    if(argc != 6)
    {
        std::cout << "Incorrect starting parameters. Must be: filename sourceIP destIP sourcePort destPort";
        return -1;
    }
        
    // work with file
    FileReader fileReader{argv[1]};

    if(!fileReader.openFile())
        return -1;

    fileReader.readFile();

    // Network configuratuion
    NetworkManager networkManager(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]));
    if(!networkManager.bindSocket())
        return -1;
    networkManager.setSockTimeout(SOCKET_WAIT_TIME_IN_SEC);

    auto transferFile = FileToTransfer(fileReader.getFileFragments(), networkManager);
    transferFile.transferFileStart();

}