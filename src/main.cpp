#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <cmath>
#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// ============================================================================
//  KONFIGURASI — ubah nilai-nilai di sini, tidak perlu mengubah body program.
// ============================================================================
namespace Config {
    // --- Window ---
    constexpr unsigned int WindowWidth  = 1280;
    constexpr unsigned int WindowHeight = 720;
    constexpr const char*  WindowTitle  = "TVG - OpenGL";
    const     glm::vec4    ClearColor   {0.1f, 0.1f, 0.12f, 1.0f};

    // --- Aset ---
    constexpr const char* VertShaderPath = "../shaders/phong.vert";
    constexpr const char* FragShaderPath = "../shaders/phong.frag";
    constexpr const char* ModelPath      = "../models/magikarp-shiny-pokemon/source/Magikarp.glb";
    // Alternatif:
    // "../models/toy-dinosaur/source/toy-dino/toy-dino.obj"
    // "../models/black_dragon.glb"

    // --- Kamera FPS (mode DEFAULT) ---
    const     glm::vec3 CameraStart {0.0f, 2.0f, 15.0f};
    constexpr float     NearPlane  = 0.1f;
    constexpr float     FarPlane   = 100.0f;

    // --- Skala & spacing model ---
    constexpr float ModelScale     = 1.0f;   // skala default di mode grid
    constexpr float ShowcaseScale  = 0.25f;  // skala model emas di mode showcase
    constexpr float ModelSpacing   = 2.5f;   // jarak antar model di grid
    constexpr float SpinSpeed      = 0.3f;   // kecepatan rotasi tiap model (mode DEFAULT)
    constexpr float ShowcaseSpin   = 0.5f;   // kecepatan rotasi model emas

    // --- Lampu ---
    const     glm::vec3 LightAmbient  {0.4f, 0.4f, 0.4f};
    const     glm::vec3 LightDiffuse  {0.8f, 0.8f, 0.8f};
    const     glm::vec3 LightSpecular {1.0f, 1.0f, 1.0f};
    constexpr float     LightOrbitRadius   = 3.0f;
    constexpr float     LightOrbitSpeed    = 0.7f;
    constexpr float     LightBaseHeight    = 1.5f;
    constexpr float     ShowcaseOrbitSpeed = 1.2f;
    constexpr float     ShowcaseBobAmp     = 0.8f;

    // --- Mode ORTHO ---
    constexpr float     OrthoSize   = 8.0f;
    const     glm::vec3 OrthoEye    {0.0f, 0.0f, 10.0f};

    // --- Mode TOPDOWN ---
    constexpr float     TopDownFov     = 60.0f;
    constexpr float     TopDownHeight  = 8.0f;

    // --- Mode SHOWCASE ---
    constexpr float     ShowcaseFov    = 45.0f;
    const     glm::vec3 ShowcaseEye    {0.0f, 1.0f, 4.0f};
}

// ============================================================================
//  Tipe data
// ============================================================================
enum SceneMode { MODE_DEFAULT, MODE_ORTHO, MODE_TOPDOWN, MODE_SHOWCASE };

struct MaterialPreset {
    const char* name;
    glm::vec3   ambient;
    glm::vec3   diffuse;
    glm::vec3   specular;
    float       shininess;   // 0..1, akan dikalikan 128 saat di-upload ke shader
};

// Tabel material klasik. Sumber: http://devernay.free.fr/cours/opengl/materials.html
constexpr std::array<MaterialPreset, 6> MATERIALS{{
    {"gold",         {0.24725f, 0.1995f,  0.0745f }, {0.75164f, 0.60648f, 0.22648f}, {0.628281f, 0.555802f, 0.366065f}, 0.4f      },
    {"silver",       {0.19225f, 0.19225f, 0.19225f}, {0.50754f, 0.50754f, 0.50754f}, {0.508273f, 0.508273f, 0.508273f}, 0.4f      },
    {"ruby",         {0.1745f,  0.01175f, 0.01175f}, {0.61424f, 0.04136f, 0.04136f}, {0.727811f, 0.626959f, 0.626959f}, 0.6f      },
    {"emerald",      {0.0215f,  0.1745f,  0.0215f }, {0.07568f, 0.61424f, 0.07568f}, {0.633f,    0.727811f, 0.633f   }, 0.6f      },
    {"cyan_plastic", {0.0f,     0.1f,     0.06f   }, {0.0f,     0.50980f, 0.50980f}, {0.50196f,  0.50196f,  0.50196f }, 0.25f     },
    {"green_rubber", {0.0f,     0.05f,    0.0f    }, {0.4f,     0.5f,     0.4f    }, {0.04f,     0.7f,      0.04f    }, 0.078125f},
}};

// ============================================================================
//  State global (dipakai callback GLFW)
// ============================================================================
Camera    camera(Config::CameraStart);
SceneMode sceneMode  = MODE_DEFAULT;
float     deltaTime  = 0.0f;
float     lastFrame  = 0.0f;
float     lastX      = Config::WindowWidth  / 2.0f;
float     lastY      = Config::WindowHeight / 2.0f;
bool      firstMouse = true;

// ============================================================================
//  Callback & input
// ============================================================================
void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoff = xpos - lastX;
    float yoff = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoff, yoff);
}

void scroll_callback(GLFWwindow*, double, double yoff) {
    camera.ProcessMouseScroll(yoff);
}

void processInput(GLFWwindow* w) {
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(w, true);
    if (glfwGetKey(w, GLFW_KEY_W)      == GLFW_PRESS) camera.ProcessKeyboard(FORWARD,  deltaTime);
    if (glfwGetKey(w, GLFW_KEY_S)      == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(w, GLFW_KEY_A)      == GLFW_PRESS) camera.ProcessKeyboard(LEFT,     deltaTime);
    if (glfwGetKey(w, GLFW_KEY_D)      == GLFW_PRESS) camera.ProcessKeyboard(RIGHT,    deltaTime);
    if (glfwGetKey(w, GLFW_KEY_1)      == GLFW_PRESS) sceneMode = MODE_DEFAULT;
    if (glfwGetKey(w, GLFW_KEY_2)      == GLFW_PRESS) sceneMode = MODE_ORTHO;
    if (glfwGetKey(w, GLFW_KEY_3)      == GLFW_PRESS) sceneMode = MODE_TOPDOWN;
    if (glfwGetKey(w, GLFW_KEY_4)      == GLFW_PRESS) sceneMode = MODE_SHOWCASE;
}

// ============================================================================
//  Helper render
// ============================================================================
float aspectRatio() {
    return static_cast<float>(Config::WindowWidth) / Config::WindowHeight;
}

void buildViewProjection(SceneMode mode, glm::mat4& view, glm::mat4& projection) {
    switch (mode) {
    case MODE_ORTHO: {
        float a = aspectRatio();
        projection = glm::ortho(-Config::OrthoSize * a,  Config::OrthoSize * a,
                                -Config::OrthoSize,      Config::OrthoSize,
                                 Config::NearPlane,      Config::FarPlane);
        view = glm::lookAt(Config::OrthoEye, glm::vec3(0.0f), glm::vec3(0, 1, 0));
        break;
    }
    case MODE_TOPDOWN: {
        projection = glm::perspective(glm::radians(Config::TopDownFov),
                                      aspectRatio(),
                                      Config::NearPlane, Config::FarPlane);
        // 0.001 di z agar lookAt tidak degenerate saat up sejajar dengan arah pandang.
        view = glm::lookAt(glm::vec3(0.0f, Config::TopDownHeight, 0.001f),
                           glm::vec3(0.0f),
                           glm::vec3(0, 1, 0));
        break;
    }
    case MODE_SHOWCASE: {
        projection = glm::perspective(glm::radians(Config::ShowcaseFov),
                                      aspectRatio(),
                                      Config::NearPlane, Config::FarPlane);
        view = glm::lookAt(Config::ShowcaseEye, glm::vec3(0.0f), glm::vec3(0, 1, 0));
        break;
    }
    case MODE_DEFAULT:
    default: {
        projection = glm::perspective(glm::radians(camera.Zoom),
                                      aspectRatio(),
                                      Config::NearPlane, Config::FarPlane);
        view = camera.GetViewMatrix();
        break;
    }
    }
}

glm::vec3 computeLightPosition(SceneMode mode, float time) {
    if (mode == MODE_SHOWCASE) {
        float s = Config::ShowcaseOrbitSpeed;
        return {
            std::sin(time * s) * Config::LightOrbitRadius,
            Config::LightBaseHeight + std::sin(time * s * 0.5f) * Config::ShowcaseBobAmp,
            std::cos(time * s) * Config::LightOrbitRadius,
        };
    }
    float s = Config::LightOrbitSpeed;
    return {
        std::sin(time * s) * Config::LightOrbitRadius,
        Config::LightBaseHeight,
        2.0f + std::cos(time * s) * 1.5f,
    };
}

void applyMaterial(const Shader& shader, const MaterialPreset& mat) {
    shader.setVec3 ("material.ambient",   mat.ambient);
    shader.setVec3 ("material.diffuse",   mat.diffuse);
    shader.setVec3 ("material.specular",  mat.specular);
    shader.setFloat("material.shininess", mat.shininess * 128.0f);
}

void uploadLight(const Shader& shader, const glm::vec3& lightPos) {
    shader.setVec3("light.position", lightPos);
    shader.setVec3("light.ambient",  Config::LightAmbient);
    shader.setVec3("light.diffuse",  Config::LightDiffuse);
    shader.setVec3("light.specular", Config::LightSpecular);
    shader.setVec3("viewPos",        camera.Position);
}

void drawScene(const Shader& shader, const Model& model, SceneMode mode, float time) {
    if (mode == MODE_SHOWCASE) {
        glm::mat4 m{1.0f};
        m = glm::rotate(m, time * Config::ShowcaseSpin, glm::vec3(0, 1, 0));
        m = glm::scale (m, glm::vec3(Config::ShowcaseScale));
        shader.setMat4("model", m);
        applyMaterial(shader, MATERIALS[0]);   // gold
        model.Draw(shader);
        return;
    }

    const int n = static_cast<int>(MATERIALS.size());
    for (int i = 0; i < n; i++) {
        glm::mat4 m{1.0f};
        m = glm::translate(m, glm::vec3((i - n / 2.0f) * Config::ModelSpacing, 0.0f, 0.0f));

        if (mode == MODE_TOPDOWN) {
            m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1, 0, 0));
        } else if (mode == MODE_DEFAULT) {
            m = glm::rotate(m, time * (i + 1) * Config::SpinSpeed, glm::vec3(0.4f, 1.0f, 0.2f));
        }
        m = glm::scale(m, glm::vec3(Config::ModelScale));
        shader.setMat4("model", m);

        applyMaterial(shader, MATERIALS[i]);
        model.Draw(shader);
    }
}

// ============================================================================
//  Entry point
// ============================================================================
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(Config::WindowWidth, Config::WindowHeight,
                                          Config::WindowTitle, nullptr, nullptr);
    if (!window) {
        std::cerr << "Gagal membuat window GLFW\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback     (window, mouse_callback);
    glfwSetScrollCallback        (window, scroll_callback);
    glfwSetInputMode             (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    Shader shader(Config::VertShaderPath, Config::FragShaderPath);
    Model  model (Config::ModelPath);

    while (!glfwWindowShouldClose(window)) {
        float now = static_cast<float>(glfwGetTime());
        deltaTime = now - lastFrame;
        lastFrame = now;

        processInput(window);

        glClearColor(Config::ClearColor.r, Config::ClearColor.g,
                     Config::ClearColor.b, Config::ClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view, projection;
        buildViewProjection(sceneMode, view, projection);

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view",       view);

        uploadLight(shader, computeLightPosition(sceneMode, now));
        drawScene  (shader, model, sceneMode, now);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
