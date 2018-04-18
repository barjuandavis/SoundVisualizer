// std library
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdio>
#include <float.h>
#include <string.h>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);
//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// for render + audio
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <fftw3.h>
#include "AudioData.h"

// Shader
#include "GLSLShader.h"
GLSLShader shader;

// nuklear (for GUI)
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "../nuklear/nuklear.h"
#include "../nuklear/nuklear_sdl_gl3.h"

#define FILE_PATH "test.wav"
#define REAL 0
#define IMAG 1

void init();
void display();
void clean();
void reshape(int width, int height);

int currFreq;
float aspect;
GLuint render_prog;

GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

// visual data
const int NUM_X = 100;
const int NUM_Z = 100;

const float SIZE_X = 6;
const float SIZE_Z = 6;
const float HALF_SIZE_X = SIZE_X / 2.0f;
const float HALF_SIZE_Z = SIZE_Z / 2.0f;

const float SPEED = 2;

glm::vec3 vertices[(NUM_X+1)*(NUM_Z+1)];
const int TOTAL_INDICES = NUM_X*NUM_Z*2*3;
GLushort indices[TOTAL_INDICES];

glm::mat4 P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

int state = 0, oldX = 0, oldY = 0;
float rX = 25, rY = -40, dist = -7;

float t = 0;

std::vector<double>* fftResult;

void init() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    shader.LoadFromFile(GL_VERTEX_SHADER, "shader.vert");
    shader.LoadFromFile(GL_FRAGMENT_SHADER, "shader.frag");

    shader.CreateAndLinkProgram();

    shader.Use();
        shader.AddAttribute("vVertex");
        shader.AddUniform("MVP");
        shader.AddUniform("t");
    shader.UnUse();
     GL_CHECK_ERRORS

    int count = 0;
    int i = 0, j = 0;

    for (j = 0; j <= NUM_Z; j++) {
        for (i = 0; i <= NUM_X; i++) {
            vertices[count++] = glm::vec3 ( ((float(i)/(NUM_X-1)) *2-1) *HALF_SIZE_X, 0, ((float(j)/(NUM_Z-1)) *2-1) *HALF_SIZE_Z);
        }
    }

    GLushort *id = &indices[0];
    for (i = 0; i < NUM_Z; i++) {
        for (j = 0; j < NUM_X; j++) {
            int i0 = i * (NUM_X + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (NUM_X + 1);
            int i3 = i2+1;

            if ( (j+i)%2) {
                *id++ = i0; *id++ = i2; *id++ = i1;
                *id++ = i1; *id++ = i2; *id++ = i3;
            } else {
                *id++ = i0; *id++ = i2; *id++ = i3;
                *id++ = i0; *id++ = i3; *id++ = i1;
            }
        }
    }

    glGenVertexArrays (1, &vaoID);
    glGenBuffers (1, &vboVerticesID);
    glGenBuffers (1, &vboIndicesID);

    glBindVertexArray (vaoID);

        glBindBuffer (GL_ARRAY_BUFFER, vboVerticesID);
        glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
        GL_CHECK_ERRORS

        glEnableVertexAttribArray (shader["vVertex"]);
        glVertexAttribPointer (shader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, 0);
        GL_CHECK_ERRORS

        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
        glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
        GL_CHECK_ERRORS

    cout << "Initialization successful\n";

}

void display() {
    t = SDL_GetTicks()/1000.0f * SPEED;

    glShadeModel(GL_SMOOTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3 (0.0f, 0.0f, dist));
    glm::mat4 Rx = glm::rotate (T, rX, glm::vec3 (1.0f, 0.0f, 0.0f));
    glm::mat4 MV = glm::rotate (Rx, rY, glm::vec3 (0.0f, 1.0f, 0.0f));
    glm::mat4 MVP = P*MV;

    shader.Use();
        glUniformMatrix4fv (shader ("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform1f (shader("t"), t);
            glBindVertexArray(vaoID);
            glDrawArrays(GL_TRIANGLES, 0, TOTAL_INDICES);
            GL_CHECK_ERRORS

    shader.UnUse();

}

void clean() {
    shader.DeleteShaderProgram();
    glDeleteVertexArrays(1, &vaoID);
    glDeleteBuffers(1, &vboIndicesID);
    glDeleteBuffers(1, &vboVerticesID);
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    aspect = float(height) / float(width);
}

void visualize(std::vector<double>* vis, int freq) {
    int bucket = 6;
    double nyquist = freq/2;
    //printf("%f",nyquist);

    double freq_bin[] = {19.0, 140.0, 400.0, 2600.0, 5200.0, nyquist};
    double peakMaxArray[bucket] {DBL_MIN};
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

    if (audio->length == 0)
        return;
    Uint32 length = (Uint32) streamLength;
    length = (length > audio->length ? audio->length : length);

    SDL_memcpy(stream, audio->pos, length);

    short signal[length];

    for (int i=0; i<length; i++)
        signal[i] = (short) *(stream+i);

    fftResult = calculateFFT(signal,(double)length); //calculateFFT is in this file :)

    audio->pos += length;
    audio->length -= length;
}


int main(int argc, char *argv[])  {

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GLContext maincontext; /* Our opengl context handle */

    AudioData audio = AudioData(FILE_PATH);

    /* Request opengl 4.4 context. */
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);

    /* Turn on double buffering with a 24bit Z buffer.
    * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *mainwindow; /* Our window handle */
    /* Create our window centered at 512x512 resolution */
    mainwindow = SDL_CreateWindow("HAHAHA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |SDL_WINDOW_RESIZABLE);

    if (!mainwindow) { /* Die if creation failed */
        std::cout << "SDL Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    /* Create our opengl context and attach it to our window */
    maincontext = SDL_GL_CreateContext(mainwindow);

    GLenum rev;
    glewExperimental = GL_TRUE;
    rev = glewInit();

    if (GLEW_OK != rev) {
        std::cout << "Error: " << glewGetErrorString(rev) << std::endl;
        exit(1);
    } else {
        std::cout << "GLEW Init: Success!" << std::endl;
    }

    SDL_GL_SetSwapInterval(1);

    bool quit=false;

    init();
    reshape(512,512);

    SDL_Event event;
    audio.wavSpec.callback = audioCallback;
    audio.wavSpec.userdata = &audio;
    audio.loadDevice();
     //PLAY HARUS DIPAKE DILUAR CALLBACK OPENGL!!!
    SDL_PauseAudioDevice(audio.device, 0);
    ///OPENGL CALLBACK
    while(!quit && audio.length > 0) {

        while( SDL_PollEvent( &event ) ) {
            if( event.type == SDL_QUIT ) {
                quit = true;
            }
            if(event.type ==SDL_WINDOWEVENT) {
                if(event.window.event = SDL_WINDOWEVENT_SIZE_CHANGED) {
                    int w,h;
                    SDL_GetWindowSize(mainwindow,&w,&h);
                    reshape(w,h);
                }
            }
        }

        display();
        SDL_GL_SwapWindow(mainwindow);
    }

    clean();
    SDL_Delay(100);

    /* Delete our opengl context, destroy our window, and shutdown SDL */
    audio.~AudioData();
    SDL_GL_DeleteContext(maincontext);
    SDL_DestroyWindow(mainwindow);
    SDL_Quit();


    return 0;
}
