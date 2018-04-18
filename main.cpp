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

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// for render + audio
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <fftw3.h>
#include "AudioData.h"

#include "LoadShaders.h"

extern GLuint LoadShaders(ShaderInfo *shaderinfo);

// nuklear (for GUI)
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
<<<<<<< HEAD
#define NK_GLFW_GL_IMPLEMENTATION
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_glfw_gl3.h"
=======
#define NK_SDL_GL2_IMPLEMENTATION
#include "../nuklear/nuklear.h"
#include "../nuklear/nuklear_sdl_gl2.h"
>>>>>>> 58e724a0b19d2b75ae18c76fb9d25ca1e74a3c81

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
GLuint vao;
GLuint vbo;
GLuint ebo;

GLint render_model_matrix_loc;
GLint render_projection_matrix_loc;
std::vector<double>* fftResult;

void init() {
    ShaderInfo shader_info[] = {
        { GL_VERTEX_SHADER, "primitive_restart.vs.glsl" },
        { GL_FRAGMENT_SHADER, "primitive_restart.fs.glsl" },
        { GL_NONE, NULL }
    };

    render_prog = LoadShaders(shader_info);
    glUseProgram(render_prog);

    render_model_matrix_loc = glGetUniformLocation(render_prog, "model_matrix");
    render_projection_matrix_loc = glGetUniformLocation(render_prog, "projection_matrix");

    // A single triangle
    static const GLfloat vertex_positions[] = {
        -1.0f, -1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  0.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 1.0f,
    };

    // Color for each vertex
    static const GLfloat vertex_colors[] = {
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f
    };

    // Indices for the triangle strips
    static const GLushort vertex_indices[] = {
        0, 1, 2
    };

    // Set up the element array buffer
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);

    // Set up the vertex attributes
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions) + sizeof(vertex_colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_positions), vertex_positions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex_positions), sizeof(vertex_colors), vertex_colors);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)sizeof(vertex_positions));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glClearColor(0.5f, 0.7f, 0.5f, 1.0f);

}

void display() {

    glm::mat4 model_matrix;

    // Setup
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate simple shading program
    glUseProgram(render_prog);

    // Set up the model and projection matrix
    glm::mat4 projection_matrix(glm::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 500.0f));
    glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

    // Set up for a glDrawElements call
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // Draw Arrays...
    model_matrix = glm::translate(glm::mat4(1.0f),glm::vec3(-3.0f, 0.0f, -5.0f));
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // DrawElements
    model_matrix = glm::translate(glm::mat4(1.0f),glm::vec3(-1.0f, 0.0f, -5.0f));
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, NULL);

    // DrawElementsBaseVertex
    model_matrix = glm::translate(glm::mat4(1.0f),glm::vec3(1.0f, 0.0f, -5.0f));
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, NULL, 1);

    // DrawArraysInstanced
    model_matrix = glm::translate(glm::mat4(1.0f),glm::vec3(3.0f, 0.0f, -5.0f));
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
}

void clean() {
    glUseProgram(0);
    glDeleteProgram(render_prog);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
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

    SDL_Window *mainwindow; /* Our window handle */
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

    /* Create our window centered at 512x512 resolution */
    mainwindow = SDL_CreateWindow("HAHAHA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |SDL_WINDOW_RESIZABLE);
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

        display();
        SDL_GL_SwapWindow(mainwindow);

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
