extern bool WriteBinaryFile(const char* fileName, TDYNAMICARRAY<char>& data);
template<typename TROOT>
void WriteToBinaryBuffer(TROOT& root, TDYNAMICARRAY<char>& output)
{
    BinaryWrite(root, output);
}

// Write a structure to a binary file
template<typename TROOT>
bool WriteToBinaryFile(TROOT& root, const char* fileName)
{
    TDYNAMICARRAY<char> out;
    WriteToBinaryBuffer(root, out);

    if (!WriteBinaryFile(fileName, out))
    {
        DFS_LOG("Could not write file %s", fileName);
        return false;
    }

    return true;
}
