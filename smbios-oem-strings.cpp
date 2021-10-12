#include <windows.h>
#include <stdio.h>

// trigger new build

this will fail...

struct RawSMBIOSData
{
    BYTE    Used20CallingMethod;
    BYTE    SMBIOSMajorVersion;
    BYTE    SMBIOSMinorVersion;
    BYTE    DmiRevision;
    DWORD    Length;
    BYTE    SMBIOSTableData[];
};

struct SMBIOSStruct {
    BYTE    Type;
    BYTE    Length;
    WORD    Handle;
};

#define MAX_STRINGS 100
char* oem_strings[MAX_STRINGS];

int setup_strings(char* s, int cstr) {
    int count = 0;
    while (count < cstr && count < MAX_STRINGS) {
        oem_strings[count] = s;
        count++;
        s = s + strlen(s) + 1;
    }
    return count;
}

int _isnum(char* s) {
    while (*s) {
        if (!isdigit(*s)) return 0;
        s++;
    }
    return 1;
}

// from https://github.com/saibulusu/SMBIOS-Parser/blob/master/Functions.cpp
SMBIOSStruct* getNextStruct(SMBIOSStruct* curStruct) {
    char* strings_begin = (char*)curStruct + curStruct->Length;
    char* next_strings = strings_begin + 1;
    while (*strings_begin != NULL || *next_strings != NULL) {
        ++strings_begin;
        ++next_strings;
    }
    return (SMBIOSStruct*)(next_strings + 1);
}

int main(int ac, char** av)
{
    DWORD error = ERROR_SUCCESS;
    DWORD smBiosDataSize = 0;
    RawSMBIOSData* smBiosData = NULL; // Defined in this link
    DWORD bytesWritten = 0;
    char* s, * e;
    struct SMBIOSStruct* d;
    int sz = 0;
    int c, i, l, n;

    // Query size of SMBIOS data.
    smBiosDataSize = GetSystemFirmwareTable('RSMB', 0, NULL, 0);

    // Allocate memory for SMBIOS data
    smBiosData = (RawSMBIOSData*)HeapAlloc(GetProcessHeap(), 0, smBiosDataSize);
    if (!smBiosData) {
        error = ERROR_OUTOFMEMORY;
        goto exit;
    }

    // Retrieve the SMBIOS table
    bytesWritten = GetSystemFirmwareTable('RSMB', 0, smBiosData, smBiosDataSize);

    if (bytesWritten != smBiosDataSize) {
        error = ERROR_INVALID_DATA;
        goto exit;
    }
    s = (char*)smBiosData->SMBIOSTableData;
    e = s + smBiosData->Length;
    while (s < e) {
        d = (struct SMBIOSStruct*)s;
        if (d->Type == 11) {
            c = s[4];
            l = setup_strings(&s[5], c);
            if (l <= c) {
                if (ac < 2) {
                    for (i = 0; i < l; i++) {
                        printf("%d: %s\n", i + 1, oem_strings[i]);
                    }
                }
                else {
                    if (strcmp("count", av[1]) == 0) {
                        printf("%d", l);
                    }
                    else if (_isnum(av[1])) {
                        n = atoi(av[1]);
                        if (n < l) {
                            printf("%s", oem_strings[n]);
                        }
                    }
                }
            }

        }
        s = (char*)getNextStruct((SMBIOSStruct*)s);
    }

exit: if (smBiosData) {
        HeapFree(GetProcessHeap(), 0, smBiosData);
      }
}
