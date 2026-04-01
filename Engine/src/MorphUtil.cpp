#include "Common/MorphUtil.h"
#include <fstream>
#include <sys/stat.h>

char CurWorkDir[256];

bool ReadFile(const char* pFileName, std::string& outFile)
{
    std::ifstream f(pFileName);
    if (!f.is_open()) {
        fprintf(stderr, "Unable to open file '%s'\n", pFileName);
        return false;
    }
    std::string line;
    while (std::getline(f, line)) {
        outFile.append(line);
        outFile.append("\n");
    }
    f.close();
    return true;
}


#ifdef _WIN32
char* ReadBinaryFile(const char* pFileName, int& size)
{
    FILE* f = NULL;
    errno_t err = fopen_s(&f, pFileName, "rb");
    if (!f) {
        fprintf(stderr, "Error opening '%s'\n", pFileName);
        exit(0);
    }
    struct stat stat_buf;
    stat(pFileName, &stat_buf);
    size = stat_buf.st_size;
    char* p = (char*)malloc(size);
    fread(p, 1, size, f);
    fclose(f);
    return p;
}
#else
char* ReadBinaryFile(const char* pFileName, int& size)
{
    FILE* f = fopen(pFileName, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open binary file '%s'\n", pFileName);
        return nullptr;
    }
    fseek(f, 0, SEEK_END);
    size = (int)ftell(f);
    rewind(f);
    char* buf = (char*)malloc(size);
    fread(buf, 1, size, f);
    fclose(f);
    return buf;
}
#endif

void WriteBinaryFile(const char* pFileName, const void* pData, int size)
{
    FILE* f = fopen(pFileName, "wb");
    if (!f) {
        fprintf(stderr, "Cannot write binary file '%s'\n", pFileName);
        exit(0);
    }
    fwrite(pData, 1, size, f);
    fclose(f);
}