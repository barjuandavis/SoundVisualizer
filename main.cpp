#include <iostream>
#include <SDL2/SDL.h>
#include <fftw3.h>
#include <cmath>
#include <vector>
#include <cstdio>
#include <float.h>
#include "AudioData.h"
#include "nuklear/nuklear.h"
#define FILE_PATH "test2.wav"
#define REAL 0
#define IMAG 1

int currFreq;


void visualize(std::vector<double>* vis, int freq) {
    int bucket = 6;
    double nyquist = freq/2;
    //printf("%f",nyquist);

    double freq_bin[] = {19.0, 140.0, 400.0, 2600.0, 5200.0, nyquist};
    double peakMaxArray[bucket]{DBL_MIN};
    double peakMax = DBL_MIN;
    int peakMaxIndex = 0;

    for (int i = 1; i<vis->size(); i++) {
        double whatFreq = i * (double )freq / (double)vis->size();
        for (int j = 0; j < bucket; j++) {
            if ((whatFreq > freq_bin[j])&&(whatFreq < freq_bin[j+1])) {
                if (vis->at(i) > peakMaxArray[j]) {
                    peakMaxArray[j] = vis->at(i);
                }
            }
        }

        if (vis->at(i) > peakMax) {
                peakMax = vis->at(i);
                peakMaxIndex = i;
        }
    }


    for (int i=0; i<bucket-1; i++) {
        peakMaxArray[i] = 10.0*log10(peakMaxArray[i]);
        int powerup = (int) peakMaxArray[i] - 7;
        for (int j = 0; j < powerup; j++) {
            printf(">");
        }
        printf("\n");
    }



  printf("\n");

}

std::vector<double>* calculateFFT(short signal[], double sampleLength) {
    const int FFT_Points = 1024;
    double maxFFTSample;
    int peakPosition;

    double temp;
    fftw_complex out[FFT_Points];
    fftw_complex complexSignal[FFT_Points];

    std::vector<double>* absSignal = new std::vector<double>(FFT_Points/2);

    for (int i=0; i<FFT_Points; i++) {
        temp = (double) ((signal[2*i] & 0xFF) | (signal[2*i+1] << 8)) / sampleLength;
        complexSignal[i][REAL] = temp;
        complexSignal[i][IMAG] = 0.0f;
    }

    fftw_plan plan;
    plan = fftw_plan_dft_1d(FFT_Points,complexSignal,out,FFTW_FORWARD,FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);

    maxFFTSample = -DBL_MAX;
    peakPosition = 0;

    for (int i=1; i<(FFT_Points/2); ++i) {

            absSignal->at(i) = sqrt(pow(out[i][REAL],2.0)+pow(out[i][IMAG],2.0));
         //   printf("%d %f\n",i,absSignal->at(i));
            if (absSignal->at(i) > maxFFTSample) {
                    maxFFTSample = absSignal->at(i);
                    peakPosition = i;
            }
    }

    return absSignal;
}

void audioCallback(void* userdata, Uint8* stream, int streamLength) {
     AudioData* audio = (AudioData*)userdata;

    if (audio->length == 0) return;
    Uint32 length = (Uint32) streamLength;
    length = (length > audio->length ? audio->length : length);

    SDL_memcpy(stream, audio->pos, length);

    short signal[length];

    for (int i=0; i<length; i++) signal[i] = (short) *(stream+i);

    std::vector<double>* fftResult = calculateFFT(signal,(double)length); //calculateFFT is in this file :)

    visualize(fftResult, audio->getFreq());

    audio->pos += length;
    audio->length -= length;
}


int main(int argc, char *argv[])  {

    AudioData audio = AudioData(FILE_PATH);
    audio.wavSpec.callback = audioCallback;
    audio.wavSpec.userdata = &audio;
    audio.loadDevice();
    audio.play();
    audio.~AudioData();

    return 0;
}
