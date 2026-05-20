#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

const unsigned int SCR_W = 1280, SCR_H = 720;

// --- Camera & timing globals (untuk callback) ---
Camera camera(glm::vec3(0.0f, 2.0f, 15.0f));
float deltaTime = 0.0f, lastFrame = 0.0f;
float lastX = SCR_W / 2.0f, lastY = SCR_H / 2.0f;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoff = xpos - lastX, yoff = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoff, yoff);
}
void scroll_callback(GLFWwindow*, double, double yoff) { camera.ProcessMouseScroll(yoff); }

enum SceneMode { MODE_DEFAULT, MODE_ORTHO, MODE_TOPDOWN, MODE_SHOWCASE };
SceneMode sceneMode = MODE_DEFAULT;

void processInput(GLFWwindow* w) {
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(w, true);
    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD,  deltaTime);
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT,     deltaTime);
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT,    deltaTime);
    if (glfwGetKey(w, GLFW_KEY_1) == GLFW_PRESS) sceneMode = MODE_DEFAULT;
    if (glfwGetKey(w, GLFW_KEY_2) == GLFW_PRESS) sceneMode = MODE_ORTHO;
    if (glfwGetKey(w, GLFW_KEY_3) == GLFW_PRESS) sceneMode = MODE_TOPDOWN;
    if (glfwGetKey(w, GLFW_KEY_4) == GLFW_PRESS) sceneMode = MODE_SHOWCASE;
}

struct MaterialPreset {
    const char* name;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;  // 0..1, dikali 128 oleh kita
};


// Nilai diambil dari http://devernay.free.fr/cours/opengl/materials.html
MaterialPreset MATERIALS[] = {
    {"gold",            {0.24725f, 0.1995f,   0.0745f },  {0.75164f, 0.60648f, 0.22648f}, {0.628281f, 0.555802f, 0.366065f}, 0.4f},
    {"silver",          {0.19225f, 0.19225f,  0.19225f},  {0.50754f, 0.50754f, 0.50754f}, {0.508273f, 0.508273f, 0.508273f}, 0.4f},
    {"ruby",            {0.1745f,  0.01175f,  0.01175f},  {0.61424f, 0.04136f, 0.04136f}, {0.727811f, 0.626959f, 0.626959f}, 0.6f},
    {"emerald",         {0.0215f,  0.1745f,   0.0215f },  {0.07568f, 0.61424f, 0.07568f}, {0.633f,    0.727811f,0.633f},     0.6f},
    {"cyan_plastic",    {0.0f,     0.1f,      0.06f   },  {0.0f,     0.50980f, 0.50980f}, {0.50196f,  0.50196f, 0.50196f},   0.25f},
    {"black_rubber",    {0.02f,    0.02f,     0.02f   },  {0.01f,    0.01f,    0.01f},    {0.4f,      0.4f,     0.4f},       0.078125f},
};
const int NUM_MATERIALS = sizeof(MATERIALS) / sizeof(MATERIALS[0]);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "TVG - OpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);   // PENTING untuk 3D

    Shader shader("../shaders/phong.vert", "../shaders/phong.frag");
    // Model myModel("../models/magikarp-shiny-pokemon/source/Magikarp.glb");
    // Model myModel("../models/toy-dinosaur/source/toy-dino/toy-dino.obj");
    Model myModel("../models/black_dragon.glb");

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    while (!glfwWindowShouldClose(window)) {
        float now = glfwGetTime();
        deltaTime = now - lastFrame; lastFrame = now;
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        // glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
        //                                         (float)SCR_W / SCR_H, 0.1f, 100.0f);
        // glm::mat4 view  = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        // Rotasi pelan supaya kelihatan kubus 3D (bukan persegi).
        // Sumbu (0.5, 1.0, 0.0) bikin rotasi miring → 3 sisi sekaligus kelihatan.
        glm::mat4 projection, view;

        switch (sceneMode) {
        case MODE_DEFAULT: {
            // Konfigurasi 1: perspective, kamera FPS bebas (WASD/mouse)
            projection = glm::perspective(glm::radians(camera.Zoom),
                                        (float)SCR_W / SCR_H, 0.1f, 100.0f);
            view = camera.GetViewMatrix();
            break;
        }
        case MODE_ORTHO: {
            // Konfigurasi 2: orthographic — proyeksi paralel (no foreshortening)
            float orthoSize = 8.0f;
            float aspect = (float)SCR_W / SCR_H;
            projection = glm::ortho(-orthoSize * aspect, orthoSize * aspect,
                                    -orthoSize, orthoSize, 0.1f, 100.0f);
            view = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0), glm::vec3(0, 1, 0));
            break;
        }
        case MODE_TOPDOWN: {
            // Konfigurasi 3: kamera dari atas, model di-rotate
            projection = glm::perspective(glm::radians(60.0f),
                                        (float)SCR_W / SCR_H, 0.1f, 100.0f);
            view = glm::lookAt(glm::vec3(0, 8, 0.001f),   // dari atas (0.001 biar lookAt tdk degenerate)
                            glm::vec3(0, 0, 0),
                            glm::vec3(0, 1, 0));
            break;
        }
        case MODE_SHOWCASE: {
            // Konfigurasi 4: showcase — 1 model emas, kamera cinematic fixed,
            // model & lampu sama-sama berputar untuk pamer specular highlight.
            projection = glm::perspective(glm::radians(45.0f),
                                        (float)SCR_W / SCR_H, 0.1f, 100.0f);
            view = glm::lookAt(glm::vec3(0.0f, 1.0f, 4.0f),
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        }
        }
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);

        // Lampu mengorbit objek → highlight specular menyapu permukaan = terlihat shiny.
        if (sceneMode == MODE_SHOWCASE) {
            // Orbit penuh 360° di radius besar supaya highlight emas jelas menyapu.
            float t = (float)glfwGetTime();
            lightPos.x = sin(t * 1.2f) * 3.0f;
            lightPos.y = 1.5f + sin(t * 0.6f) * 0.8f;
            lightPos.z = cos(t * 1.2f) * 3.0f;
        } else {
            lightPos.x = sin(glfwGetTime() * 0.7f) * 3.0f;
            lightPos.y = 1.5f;
            lightPos.z = 2.0f + cos(glfwGetTime() * 0.7f) * 1.5f;
        }

        // Light — full white intensity, sesuai asumsi tabel preset material klasik.
        // Kalau diffuse/ambient < 1.0, warna material dipotong → emas tidak kelihatan keemasan.
        shader.setVec3("light.position", lightPos);
        shader.setVec3("light.ambient",  0.4f, 0.4f, 0.4f);
        shader.setVec3("light.diffuse",  0.8f, 0.8f, 0.8f);
        shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        shader.setVec3("viewPos", camera.Position);

        if (sceneMode == MODE_SHOWCASE) {
            // Showcase: 1 model emas di center, berputar di sumbu Y (yaw spin).
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::rotate(m, (float)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::scale(m, glm::vec3(0.25f));
            shader.setMat4("model", m);

            const auto& mat = MATERIALS[0]; // gold
            shader.setVec3 ("material.ambient",   mat.ambient);
            shader.setVec3 ("material.diffuse",   mat.diffuse);
            shader.setVec3 ("material.specular",  mat.specular);
            shader.setFloat("material.shininess", mat.shininess * 128.0f);

            myModel.Draw(shader);
        } else {
            for (int i = 0; i < NUM_MATERIALS; i++) {
                glm::mat4 m = glm::mat4(1.0f);
                m = glm::translate(m, glm::vec3((i - NUM_MATERIALS / 2.0f) * 2.5f, 0.0f, 0.0f));

                if (sceneMode == MODE_TOPDOWN) {
                    // putar model agar wajah depan menghadap atas
                    m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1, 0, 0));
                }
                if (sceneMode == MODE_DEFAULT) {
                    // rotasi animasi otomatis
                    m = glm::rotate(m, (float)glfwGetTime() * (i + 1) * 0.3f, glm::vec3(0.4f, 1.0f, 0.2f));
                }
                m = glm::scale(m, glm::vec3(0.02f));
                shader.setMat4("model", m);

                const auto& mat = MATERIALS[i];
                shader.setVec3 ("material.ambient",   mat.ambient);
                shader.setVec3 ("material.diffuse",   mat.diffuse);
                shader.setVec3 ("material.specular",  mat.specular);
                shader.setFloat("material.shininess", mat.shininess * 128.0f);

                myModel.Draw(shader);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}