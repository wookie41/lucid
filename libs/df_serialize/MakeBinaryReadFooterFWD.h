extern bool LoadBinaryFile(const char* fileName, TDYNAMICARRAY<char>& data);
// Read a structure from a binary string
template<typename TROOT>
bool ReadFromBinaryBuffer(TROOT& root, TDYNAMICARRAY<char>& data)
{
    size_t offset = 0;
    return BinaryRead(root, data, offset);
}

// Read a structure from a binary file
template<typename TROOT>
bool ReadFromBinaryFile(TROOT& root, const char* fileName)
{
    TDYNAMICARRAY<char> fileData;
    if (!LoadBinaryFile(fileName, fileData))
    {
        DFS_LOG("Could not read file %s", fileName);
        return false;
    }

    return ReadFromBinaryBuffer(root, fileData);
}
