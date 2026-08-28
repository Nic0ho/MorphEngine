#include "MorphPlatform.h"
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>

bool morphPlatformOpenFileDialog(char* outPath, u32 outSize, const char* filter)
{
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = outPath;
    ofn.nMaxFile = outSize;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    return GetOpenFileNameA(&ofn) != 0;
}

bool morphPlatformOpenFolderDialog(char* outPath, u32 outSize)
{
    BROWSEINFOA bi = {0};
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpszTitle = "Select project location";

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (!pidl) return false;

    SHGetPathFromIDListA(pidl, outPath);
    CoTaskMemFree((void*)pidl);
    return true;
}

bool morphPlatformRemoveDirectory(const char* path)
{
    char doubleNull[MAX_PATH_LEN + 1] = {0};
    strncpy(doubleNull, path, MAX_PATH_LEN);

    SHFILEOPSTRUCTA op = {0};
    op.wFunc = FO_DELETE;
    op.pFrom = doubleNull;
    op.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;

    return SHFileOperationA(&op) == 0;
}

bool morphPlatformRegisterFileAssociation(const char* exePath)
{
    HKEY key;
    char command[MAX_PATH_LEN * 2];

    //HKEY_CLASSES_ROOT\.mproj
    RegCreateKeyExA(HKEY_CLASSES_ROOT, ".mproj", 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, NULL);
    RegSetValueExA(key, NULL, 0, REG_SZ, 
        (BYTE*)"MorphEngine.Project", 20);
    RegCloseKey(key);

    //HKEY_CLASSES_ROOT\MorphEngine.Project
    RegCreateKeyExA(HKEY_CLASSES_ROOT, "MorphEngine.Project", 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, NULL);
    RegSetValueExA(key, NULL, 0, REG_SZ,
        (BYTE*)"Morph Engine Project", 21);
    RegCloseKey(key);

    //HKEY_CLASSES_ROOT\MorphEngine.Project\DefaultIcon
    RegCreateKeyExA(HKEY_CLASSES_ROOT, 
        "MorphEngine.Project\\DefaultIcon", 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, NULL);
    snprintf(command, sizeof(command), "\"%s\",0", exePath);
    RegSetValueExA(key, NULL, 0, REG_SZ, (BYTE*)command, strlen(command)+1);
    RegCloseKey(key);

    //HKEY_CLASSES_ROOT\MorphEngine.Project\shell\open\command
    RegCreateKeyExA(HKEY_CLASSES_ROOT,
        "MorphEngine.Project\\shell\\open\\command", 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, NULL);
    snprintf(command, sizeof(command), "\"%s\" \"%%1\"", exePath);
    RegSetValueExA(key, NULL, 0, REG_SZ, (BYTE*)command, strlen(command)+1);
    RegCloseKey(key);


    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return true;
}

bool morphPlatformCopyFile(const char* src, const char* dest)
{ return CopyFileA(src, dest, FALSE) != 0; }