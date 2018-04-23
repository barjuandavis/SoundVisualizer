#include <bits/stdc++.h>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//GLUT
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

//nuklear
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear/nuklear.h>
#include <nuklear/nuklear_glfw_gl2.h>

//Header
#include "Shader.h"
#include "Camera.h"

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int width;
int height;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

// slider data
float light = 1.0f;
float red = 0.0f;
float green = 0.0f;
float blue = 0.0f;
float specular = 0.0f;
static const char *mats[] = {"Emerald", "Jade", "Obsidian", "Pearl", "Ruby", "Turquoise", "Brass", "Bronze", "Chrome", "Copper", "Gold", "Silver", "Plastic", "Rubber"};
static int current_mats = 0;
struct material
{
    float ar = 0.0215f;
    float ag = 0.1745f;
    float ab = 0.0215f;
    float dr = 0.07586f;
    float dg = 0.61424f;
    float db = 0.07568f;
    float sr = 0.633f;
    float sg = 0.727811f;
    float sb = 0.633f;
} mat;

#define DETAIL 50
#define NUM_VERTICES DETAIL*DETAIL
#define PI 3.141592654f
#define EPS 1e-9


struct Vertex
{
    GLfloat x, y, z;
    GLfloat r, g, b;
    Vertex(GLfloat _x, GLfloat _y, GLfloat _z, GLfloat _r, GLfloat _g, GLfloat _b) :
        x(_x), y(_y), z(_z), r(_r), g(_g), b(_b) {}
    Vertex(double _x, double _y, double _z, double _r, double _g, double _b) :
        x((GLfloat)_x), y((GLfloat)_y), z((GLfloat)_z), r((GLfloat)_r), g((GLfloat)_g), b((GLfloat)_b) {}
    Vertex() {}
    Vertex(GLfloat _x, GLfloat _y, GLfloat _z) :
        x(_x), y(_y), z(_z), r(1.f), g(1.f), b(1.f) {}
    friend ostream& operator << (ostream &out, Vertex &vertex);
    bool zero()
    {
        return x < EPS && y < EPS && z < EPS;
    }
};
ostream& operator << (ostream &out, Vertex &vertex)
{
    out << vertex.x << ", " << vertex.y << ", " << vertex.z;
    return out;
}

Vertex sphereVertices[NUM_VERTICES * 4];

double powa(double base, double exp)
{
    int k = 1;
    if (fmod(exp,2) < EPS)
        k = 2;
    return pow((base < 0? -1 : 1), k)*pow(abs(base), exp);
}

void createSuperellipsoid(GLint R, GLfloat n1, GLfloat n2)
{
    for (int i = 0; i < DETAIL; ++i)
    {
        for (int j = 0; j < DETAIL; ++j)
        {
            int k = i * DETAIL + j;

            sphereVertices[k * 4] = Vertex(
                                        powa(sin(i*PI / DETAIL), n1)*powa(cos(j * 2 * PI / DETAIL), n2),
                                        powa(sin(i*PI / DETAIL), n1)*powa(sin(j * 2 * PI / DETAIL), n2),
                                        powa(cos(i*PI / DETAIL), n1),
                                        sin(i*PI / DETAIL)*cos(j * 2 * PI / DETAIL) / 2 + .5,
                                        sin(i*PI / DETAIL)*sin(j * 2 * PI / DETAIL) / 2 + .5,
                                        cos(i*PI / DETAIL) / 2 + .5
                                    );

            sphereVertices[k * 4 + 1] = Vertex(
                                            powa(sin(i*PI / DETAIL), n1)*powa(cos((j + 1) * 2 * PI / DETAIL), n2),
                                            powa(sin(i*PI / DETAIL), n1)*powa(sin((j + 1) * 2 * PI / DETAIL), n2),
                                            powa(cos(i*PI / DETAIL), n1),
                                            sin(i*PI / DETAIL)*cos((j + 1) * 2 * PI / DETAIL) / 2 + .5,
                                            sin(i*PI / DETAIL)*sin((j + 1) * 2 * PI / DETAIL) / 2 + .5,
                                            cos(i*PI / DETAIL) / 2 + .5
                                        );

            sphereVertices[k * 4 + 2] = Vertex(
                                            powa(sin((i + 1)*PI / DETAIL), n1)*powa(cos(j * 2 * PI / DETAIL), n2),
                                            powa(sin((i + 1)*PI / DETAIL), n1)*powa(sin(j * 2 * PI / DETAIL), n2),
                                            powa(cos((i + 1)*PI / DETAIL), n1),
                                            sin((i + 1)*PI / DETAIL)*cos(j * 2 * PI / DETAIL) / 2 + .5,
                                            sin((i + 1)*PI / DETAIL)*sin(j * 2 * PI / DETAIL) / 2 + .5,
                                            cos((i + 1)*PI / DETAIL) / 2 + .5
                                        );

            sphereVertices[k * 4 + 3] = Vertex(
                                            powa(sin(i*PI / DETAIL), n1)*powa(cos(j * 2 * PI / DETAIL), n2),
                                            powa(sin(i*PI / DETAIL), n1)*powa(sin(j * 2 * PI / DETAIL), n2),
                                            powa(cos(i*PI / DETAIL), n1),
                                            sin(i*PI / DETAIL)*cos(j * 2 * PI / DETAIL) / 2 + .5,
                                            sin(i*PI / DETAIL)*sin(j * 2 * PI / DETAIL) / 2 + .5,
                                            cos(i*PI / DETAIL) / 2 + .5
                                        );

        }
    }
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Slider", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    GLFWwindow* gui = glfwCreateWindow(SCR_WIDTH/2, SCR_HEIGHT/2+200, "Control", NULL, window);
    if (gui == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    struct nk_context *ctx;
    struct nk_colorf bg;
    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

    /* gui */
    ctx = nk_glfw3_init(gui, NK_GLFW3_INSTALL_CALLBACKS);

    struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();


    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    /// tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("1.materials.vs", "1.materials.fs");
    Shader lightingShader1("1.materials1.vs", "1.materials1.fs");
    Shader lampShader("1.lamp.vs", "1.lamp.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] =
    {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    createSuperellipsoid(1, 1, 1);
    // first, configure the sphere's VAO (and VBO)
    unsigned int VBO, sphereVAO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);

    glBindVertexArray(sphereVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    createSuperellipsoid(1, 1, 1);
    // first, configure the sphere's VAO (and VBO)
    unsigned int sphere1VAO;
    glGenVertexArrays(1, &sphere1VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);

    glBindVertexArray(sphere1VAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D sphere)
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // Main Window
        //------------
        glfwMakeContextCurrent(window);

        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float t = glfwGetTime()*2;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("light.position", glm::vec3(sin(light)+lightPos.x, cos(light)+lightPos.y, sin(light)+lightPos.z));
        lightingShader.setVec3("viewPos", camera.Position);

        // light properties
        lightingShader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f); // note that all light colors are set at full intensity
        lightingShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        lightingShader.setVec3("material.ambient", bg.r*mat.ar, bg.g*mat.ag, bg.b*mat.ab);
        lightingShader.setVec3("material.diffuse", bg.r*mat.dr, bg.g*mat.dg,bg.b*mat.db);
        lightingShader.setVec3("material.specular", bg.r*mat.sr, bg.g*mat.sg, bg.b*mat.sb);
        lightingShader.setFloat("material.shininess", 32.5f-specular);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        lightingShader.setFloat("t", t);

        // render the sphere
        glBindVertexArray(sphereVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERTICES*8);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader1.use();
        lightingShader1.setVec3("light.position", glm::vec3(sin(light)+lightPos.x, cos(light)+lightPos.y, sin(light)+lightPos.z));
        lightingShader1.setVec3("viewPos", camera.Position);

        // light properties
        lightingShader1.setVec3("light.ambient", 1.0f, 1.0f, 1.0f); // note that all light colors are set at full intensity
        lightingShader1.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader1.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        lightingShader1.setVec3("material.ambient", bg.r*mat.ar, bg.g*mat.ag, bg.b*mat.ab);
        lightingShader1.setVec3("material.diffuse", bg.r*mat.dr, bg.g*mat.dg,bg.b*mat.db);
        lightingShader1.setVec3("material.specular", bg.r*mat.sr, bg.g*mat.sg, bg.b*mat.sb);
        lightingShader1.setFloat("material.shininess", 32.5f-specular);

        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        lightingShader1.setMat4("projection", projection);
        lightingShader1.setMat4("view", view);

        // world transformation
        model = glm::mat4(1.0f);
        lightingShader1.setMat4("model", model);

        lightingShader1.setFloat("t", t);

        // render the sphere
        glBindVertexArray(sphere1VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERTICES*8);


        // also draw the lamp object
        lampShader.use();
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(sin(light)+lightPos.x, cos(light)+lightPos.y, sin(light)+lightPos.z+cos(light)));
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller sphere
        lampShader.setMat4("model", model);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERTICES*8);

        glfwSwapBuffers(window);

        // gui
        //------------
        glfwMakeContextCurrent(gui);
        glfwPollEvents();
        nk_glfw3_new_frame();

        if (nk_begin(ctx, "Controls", nk_rect(0, 0, SCR_WIDTH/2, SCR_HEIGHT/2+200),
                     NK_WINDOW_BORDER|NK_WINDOW_CLOSABLE))
        {
            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            {
                nk_layout_row_push(ctx, 50);
                nk_label(ctx, "Light:", NK_TEXT_LEFT);
                nk_layout_row_push(ctx, 110);
                nk_slider_float(ctx, 1, &light, 10.0f, 0.1f);
            }
            nk_layout_row_end(ctx);

            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            {
                nk_layout_row_push(ctx, 50);
                nk_label(ctx, "Specular:", NK_TEXT_LEFT);
                nk_layout_row_push(ctx, 110);
                nk_slider_float(ctx, 0, &specular, 32.0f, 1.0f);
            }
            nk_layout_row_end(ctx);

            nk_layout_row_static(ctx, 25, 200, 1);
            current_mats = nk_combo(ctx, mats, NK_LEN(mats), current_mats, 25, nk_vec2(200,200));
            switch (current_mats)
            {
            case 0:
                mat.ar = 0.0215f;
                mat.ag = 0.1745f;
                mat.ab = 0.0215f;
                mat.dr = 0.07586f;
                mat.dg = 0.61424f;
                mat.db = 0.07568f;
                mat.sr = 0.633f;
                mat.sg = 0.727811f;
                mat.sb = 0.633f;
                break;

            case 1:
                mat.ar = 0.135f;
                mat.ag = 0.2225f;
                mat.ab = 0.1575f;
                mat.dr = 0.54f;
                mat.dg = 0.89f;
                mat.db = 0.63f;
                mat.sr = 0.316228f;
                mat.sg = 0.316228f;
                mat.sb = 0.316228f;
                break;

            case 2:
                mat.ar = 0.05375f;
                mat.ag = 0.05f;
                mat.ab = 0.06625f;
                mat.dr = 0.18275f;
                mat.dg = 0.17f;
                mat.db = 0.22525f;
                mat.sr = 0.332741f;
                mat.sg = 0.328634f;
                mat.sb = 0.346435f;
                break;

            case 3:
                mat.ar = 0.25f;
                mat.ag = 0.20725f;
                mat.ab = 0.20725f;
                mat.dr = 1.0f;
                mat.dg = 0.829f;
                mat.db = 0.829f;
                mat.sr = 0.296648f;
                mat.sg = 0.296648f;
                mat.sb = 0.296648f;
                break;

            case 4:
                mat.ar = 0.1745f;
                mat.ag = 0.01175f;
                mat.ab = 0.01175f;
                mat.dr = 0.61424f;
                mat.dg = 0.04136f;
                mat.db = 0.04136f;
                mat.sr = 0.727811f;
                mat.sg = 0.626959f;
                mat.sb = 0.626959f;
                break;

            case 5:
                mat.ar = 0.1f;
                mat.ag = 0.18725f;
                mat.ab = 0.1745f;
                mat.dr = 0.396f;
                mat.dg = 0.74151f;
                mat.db = 0.69102f;
                mat.sr = 0.297254f;
                mat.sg = 0.30829f;
                mat.sb = 0.306678f;
                break;

            case 6:
                mat.ar = 0.329412f;
                mat.ag = 0.223529f;
                mat.ab = 0.027451f;
                mat.dr = 0.780392f;
                mat.dg = 0.568627f;
                mat.db = 0.113725f;
                mat.sr = 0.992157f;
                mat.sg = 0.941176f;
                mat.sb = 0.807843f;
                break;

            case 7:
                mat.ar = 0.2125f;
                mat.ag = 0.1275f;
                mat.ab = 0.054f;
                mat.dr = 0.714f;
                mat.dg = 0.4284f;
                mat.db = 0.18144f;
                mat.sr = 0.393548f;
                mat.sg = 0.271906f;
                mat.sb = 0.166721f;
                break;

            case 8:
                mat.ar = 0.25f;
                mat.ag = 0.25f;
                mat.ab = 0.25f;
                mat.dr = 0.4f;
                mat.dg = 0.4f;
                mat.db = 0.4f;
                mat.sr = 0.774597f;
                mat.sg = 0.774597f;
                mat.sb = 0.774597f;
                break;

            case 9:
                mat.ar = 0.19125f;
                mat.ag = 0.0735f;
                mat.ab = 0.0225f;
                mat.dr = 0.7038f;
                mat.dg = 0.27048f;
                mat.db = 0.0828f;
                mat.sr = 0.256777f;
                mat.sg = 0.137622f;
                mat.sb = 0.086014f;
                break;

            case 10:
                mat.ar = 0.24725f;
                mat.ag = 0.1995f;
                mat.ab = 0.0745f;
                mat.dr = 0.75614f;
                mat.dg = 0.60648f;
                mat.db = 0.22648f;
                mat.sr = 0.628281f;
                mat.sg = 0.555802f;
                mat.sb = 0.366065f;
                break;

            case 11:
                mat.ar = 0.19225f;
                mat.ag = 0.19225f;
                mat.ab = 0.19225f;
                mat.dr = 0.50754f;
                mat.dg = 0.50754f;
                mat.db = 0.50754f;
                mat.sr = 0.508273f;
                mat.sg = 0.508273f;
                mat.sb = 0.508273f;
                break;

            case 12:
                mat.ar = 0.0f;
                mat.ag = 0.0f;
                mat.ab = 0.0f;
                mat.dr = 0.55f;
                mat.dg = 0.55f;
                mat.db = 0.55f;
                mat.sr = 0.7f;
                mat.sg = 0.7f;
                mat.sb = 0.7f;
                break;

            case 13:
                mat.ar = 0.05f;
                mat.ag = 0.05f;
                mat.ab = 0.05f;
                mat.dr = 0.5f;
                mat.dg = 0.5f;
                mat.db = 0.5f;
                mat.sr = 0.7f;
                mat.sg = 0.7f;
                mat.sb = 0.7f;
                break;
            }

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "Color:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400)))
            {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }

        }
        nk_end(ctx);

        /* Draw */
        glfwGetWindowSize(gui, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
         * with blending, scissor, face culling and depth test and defaults everything
         * back into a default state. Make sure to either save and restore or
         * reset your own state after drawing rendering the UI. */
        nk_glfw3_render(NK_ANTI_ALIASING_ON);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(gui);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    nk_glfw3_shutdown();
    glfwDestroyWindow(gui);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
