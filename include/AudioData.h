#include <SDL2/SDL.h>
#include <string>
#ifndef AUDIODATA_H
#define AUDIODATA_H


class AudioData
{
    public:
        AudioData(std::string file_path);
        virtual ~AudioData();
        void setCallback(void* callback);
        double getFreq();
        void play();
        void loadDevice();
        Uint8* pos;
        Uint32 length;
        SDL_AudioSpec wavSpec;
        SDL_AudioDeviceID device;
};

#endif // AUDIODATA_H
