#include "AudioData.h"
#include <string>
#include <iostream>
#include <fftw3.h>


AudioData::AudioData(std::string file_path) {
    if (SDL_Init( SDL_INIT_AUDIO ) < 0) {
       std::cout << "Error in SDL! " << SDL_GetError() << std::endl;
    } else {
        std::cout << "Initializing SDL2 done!\n";

    }

    if(SDL_LoadWAV(file_path.c_str(), &wavSpec, &pos, &length) == NULL) {
		std::cerr << "Error: " << file_path
			<< " could not be loaded as an audio file" << std::endl;
	}else {
        std::cout << "Loading " << file_path <<" done!\n";
    }

}

void AudioData::loadDevice() {
    device = SDL_OpenAudioDevice(NULL,0,&wavSpec,NULL,SDL_AUDIO_ALLOW_ANY_CHANGE);

     if (device == 0 ){
        std::cerr << "Error : " <<SDL_GetError() << std::endl;
    } else {
        std::cout << "Opening device done!\n";
    }
}

AudioData::~AudioData() {
    fftw_cleanup();
    SDL_CloseAudioDevice(device);
    SDL_FreeWAV(pos);
    SDL_Quit();
}

void AudioData::play() {
    SDL_PauseAudioDevice(device, 0); // play audio

    while(length > 0) {
        SDL_Delay(100);
    }
}


double AudioData::getFreq() {
     return wavSpec.freq;

}
