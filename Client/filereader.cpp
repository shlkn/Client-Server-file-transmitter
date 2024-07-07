#include "filereader.h"

using namespace std;


FileReader::FileReader(string _fileName)
{
    fileName = _fileName;
}

bool FileReader::openFile()
{
    fileStream = fopen(fileName.c_str(), "rb");
    if(fileStream == nullptr)
    {
        cout << "Can`t open file for read" << endl;
        return false;
    }
    return true;  
}

void FileReader::readFile()
{
    int retVal;
    do
    {
        uint32_t segmentNumber = fileFragments.size() + 1;
        auto fileSegment = std::make_pair(segmentNumber, FileFragment());
        retVal = fread(&fileSegment.second.fileData, sizeof(uint8_t), MAX_DATA_SIZE, fileStream);

        if(retVal > 0)
        {
            fileSegment.second.actualSizeOfFileData = retVal;
            fileFragments.insert(std::move(fileSegment));
        }
        else
            break;

    }while(true);
}

std::unordered_map<uint32_t, struct FileFragment> FileReader::getFileFragments()
{
    return fileFragments;
}

FileReader::~FileReader()
{
    fclose(fileStream);
}