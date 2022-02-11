#include <iostream>
#include <fstream>
#include <fftw3.h>

int main(int argv, char** argc) {
    std::ifstream f ("yeah.wav", std::ifstream::in);

    void* buf = malloc(4096*2*2);
    char* metadata = (char*) buf;

    f.read(metadata, 44);
    if (*(int*)(metadata) != 0x46464952) {
        printf("RIFF header invalid\n");
        exit(1);
    }
    if (*(int*)(metadata + 8) != 0x45564157) {
        printf("not wave file\n");
        exit(1);
    }
    if (*(char*)(metadata + 20) != 1) {
        printf("not PCM\n");
        exit(1);
    }
    int sampleRate = (*(int*)(metadata+24));
    printf("sample rate: %i\n", sampleRate);

    if (*(char*)(metadata + 34) != 16) {
        printf("not 16bit depth\n");
        exit(1);
    }

    if (*(char*)(metadata + 36) != 0x61746164) {
        printf("very confused\n");
        exit(1);
    }
    int dataSize = *(int*)(metadata+40);
    void* converted_samples = fftw_malloc(dataSize);
    char* data = (char*) buf;
    int16_t* samples = (int16_t*)data;

    return 0;
}
