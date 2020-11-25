#include "main.h"

#include <stdio.h>

// a sample exported function
int DLL_EXPORT SomeFunction(char* filename, char** buffer, long int *bufferSize)
{
    FILE        *f = NULL;
    char        *_buffer = NULL;
    long int    _bufferSize = 0;

    f = fopen(filename, "rb+");
    if(f == NULL)
    {
        return -1;
    }
    fseek(f, 0, SEEK_END);
    _bufferSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    _buffer = (char*)calloc(_bufferSize, sizeof(char));
    if(_buffer == NULL)
    {
        return -1;
    }
    fread(_buffer, sizeof(char), _bufferSize, f);

    *buffer     = _buffer;
    *bufferSize = _bufferSize;
    return 0;
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
