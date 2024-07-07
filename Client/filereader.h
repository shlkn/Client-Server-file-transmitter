#ifndef FILE_READER
#include <stdio.h>
#include <iostream>
#include <string>
#include <memory>
#include <array>
#include "transferfilemanager.h"

class FileReader
{
public:
    FileReader(std::string _fileName);
    ~FileReader();
    bool openFile();
    void readFile();
    std::unordered_map<uint32_t, struct FileFragment> getFileFragments();

private:
    std::unordered_map<uint32_t, struct FileFragment> fileFragments;
    std::string fileName;
    FILE * fileStream;
};
#endif