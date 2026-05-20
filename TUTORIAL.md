# Tutorial End-to-End — Tugas OpenGL (Phong + Material + Model Loading + MVP)

> Tutorial ini berfokus pada **eksekusi tugas**. Untuk teori lengkap (kenapa-nya), lihat [OPENGL_GUIDE.md](./OPENGL_GUIDE.md). Setiap fase di sini menyebut **section guide** yang relevan kalau Anda mau mendalami.
>
> Posisi awal: project Anda sekarang menampilkan **segitiga 2D hijau** dengan shader inline di `src/main.cpp`. Akhir tutorial: scene 3D dengan model `.obj` ber-lighting Phong, beberapa material berbeda, dan tiga setup MVP berbeda untuk screenshot.

---

## Daftar Isi

- [Ringkasan Tugas → Pemetaan Fase](#ringkasan-tugas--pemetaan-fase)
- [Fase 0 — Setup Dependencies (GLM, Assimp, stb_image)](#fase-0--setup-dependencies-glm-assimp-stb_image)
- [Fase 1 — Restrukturisasi Project & Class `Shader`](#fase-1--restrukturisasi-project--class-shader)
- [Fase 2 — MVP Matriks & Class `Camera`](#fase-2--mvp-matriks--class-camera)
- [Fase 3 — Vertex & Fragment Shader Phong](#fase-3--vertex--fragment-shader-phong)
- [Fase 4 — Sistem Material (Emas, Ruby, Plastik, dll.)](#fase-4--sistem-material-emas-ruby-plastik-dll)
- [Fase 5 — Load Model `.obj` (Class `Mesh` + `Model`)](#fase-5--load-model-obj-class-mesh--model)
- [Fase 6 — Tiga Konfigurasi MVP untuk Screenshot](#fase-6--tiga-konfigurasi-mvp-untuk-screenshot)
- [Fase 7 — Menyiapkan Deliverables (Report + Screenshot)](#fase-7--menyiapkan-deliverables-report--screenshot)
- [Appendix — Troubleshooting Cepat](#appendix--troubleshooting-cepat)

---

## Ringkasan Tugas → Pemetaan Fase

| Tugas | Diselesaikan di | Section Guide |
|---|---|---|
| Download 3D model `.obj` | Fase 5 (langkah 5.0) | — |
| Vertex + Fragment Shader (Phong) | Fase 3 | [2.2 Basic Lighting](./OPENGL_GUIDE.md#22-basic-lighting-phong-model) |
| Play around with shaders | Fase 3 (eksperimen warna/shininess), Fase 4 | [1.5 Shaders & GLSL](./OPENGL_GUIDE.md#15-shaders--glsl) |
| Various materials | Fase 4 | [2.3 Materials](./OPENGL_GUIDE.md#23-materials) |
| Fungsi load model + VBO/VAO di GPU | Fase 5 | [3.1–3.3 Model Loading](./OPENGL_GUIDE.md#31-assimp) |
| Setup berbeda untuk M, V, P | Fase 2 + Fase 6 | [1.8 Coordinate Systems](./OPENGL_GUIDE.md#18-coordinate-systems-mvp) |
| Report + screenshot | Fase 7 | — |

**Estimasi waktu:** ~6–10 jam total (paralelkan antar anggota tim: 1 orang fokus shader+material, 1 orang model loading, 1 orang camera+report).

---

## Fase 0 — Setup Dependencies (GLM, Assimp, stb_image)

### 0.1 Install library system

```bash
sudo apt update
sudo apt install libglm-dev libassimp-dev
```

- **GLM** — math library (vec3, mat4, perspective, lookAt). Header-only.
- **Assimp** — parser file 3D (.obj, .fbx, .gltf, dll). Akan kita pakai di Fase 5.
- **stb_image** — load PNG/JPG. Tidak ada di apt; kita download manual (single header).

### 0.2 Download `stb_image.h`

```bash
mkdir -p /media/adi/CodingSpace/Kuliah/TVG/include
wget -O /media/adi/CodingSpace/Kuliah/TVG/include/stb_image.h \
  https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
```

### 0.3 Update `CMakeLists.txt`

Ganti seluruh isi `CMakeLists.txt` menjadi:

```cmake
cmake_minimum_required(VERSION 3.10)
project(TVG_OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- GLAD ---
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

# --- Dependencies ---
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)

# --- Executable ---
add_executable(app
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/stb_image_impl.cpp
)
target_include_directories(app PRIVATE include src)
target_link_libraries(app PRIVATE glad glfw dl glm::glm assimp)
```

> **Kenapa `stb_image_impl.cpp` jadi file terpisah?** `stb_image.h` adalah single-header. Implementasinya hanya muncul kalau ada satu file yang `#define STB_IMAGE_IMPLEMENTATION` sebelum include. Kalau di-define di banyak file → linker error symbol ganda.

### 0.4 Bikin file kerangka

Jalankan dari root project:

```bash
mkdir -p src include shaders models assets
touch src/Shader.h src/Shader.cpp
touch src/Camera.h src/Camera.cpp
touch src/Mesh.h src/Mesh.cpp
touch src/Model.h src/Model.cpp
touch src/stb_image_impl.cpp
touch shaders/phong.vert shaders/phong.frag
```

Isi `src/stb_image_impl.cpp` cukup 2 baris:

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
```

### 0.5 Tes build masih jalan

```bash
cd build && cmake .. && make -j$(nproc)
```

Build harus sukses meski file `.cpp` baru kosong (asal sudah ada `#include` yang minimal). Kalau error "undefined reference", lanjutkan ke Fase 1 — file akan diisi.

> 📖 **Guide:** Section [1.2](./OPENGL_GUIDE.md#12-creating-a-window-glfw--glad) untuk setup GLFW/GLAD, dan Tools table di [0.3](./OPENGL_GUIDE.md#03-tools-yang-akan-kita-pakai) untuk peran masing-masing library.

---

## Fase 1 — Restrukturisasi Project & Class `Shader`

**Tujuan:** keluarkan shader source dari string inline, baca dari file `.vert`/`.frag`. Ini wajib karena Phong shader nanti panjang (60+ baris).

### 1.1 `src/Shader.h`

```cpp
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void use() const { glUseProgram(ID); }

    void setBool (const std::string& n, bool v)              const;
    void setInt  (const std::string& n, int v)               const;
    void setFloat(const std::string& n, float v)             const;
    void setVec3 (const std::string& n, const glm::vec3& v)  const;
    void setVec3 (const std::string& n, float x, float y, float z) const;
    void setMat4 (const std::string& n, const glm::mat4& m)  const;

private:
    static void checkCompile(unsigned int shader, const std::string& type);
};
```

### 1.2 `src/Shader.cpp`

```cpp
#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

static std::string readFile(const char* path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "Failed to open: " << path << "\n"; return ""; }
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vSrc = readFile(vertexPath);
    std::string fSrc = readFile(fragmentPath);
    const char* vCode = vSrc.c_str();
    const char* fCode = fSrc.c_str();

    unsigned int v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vCode, nullptr); glCompileShader(v);
    checkCompile(v, "VERTEX");

    unsigned int f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fCode, nullptr); glCompileShader(f);
    checkCompile(f, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, v); glAttachShader(ID, f);
    glLinkProgram(ID);
    checkCompile(ID, "PROGRAM");

    glDeleteShader(v); glDeleteShader(f);
}

void Shader::checkCompile(unsigned int s, const std::string& type) {
    int ok; char log[1024];
    if (type == "PROGRAM") {
        glGetProgramiv(s, GL_LINK_STATUS, &ok);
        if (!ok) { glGetProgramInfoLog(s, 1024, nullptr, log);
                   std::cerr << "PROGRAM LINK ERROR:\n" << log << "\n"; }
    } else {
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { glGetShaderInfoLog(s, 1024, nullptr, log);
                   std::cerr << type << " COMPILE ERROR:\n" << log << "\n"; }
    }
}

void Shader::setBool (const std::string& n, bool v)             const { glUniform1i (glGetUniformLocation(ID, n.c_str()), (int)v); }
void Shader::setInt  (const std::string& n, int v)              const { glUniform1i (glGetUniformLocation(ID, n.c_str()), v); }
void Shader::setFloat(const std::string& n, float v)            const { glUniform1f (glGetUniformLocation(ID, n.c_str()), v); }
void Shader::setVec3 (const std::string& n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(ID, n.c_str()), 1, &v[0]); }
void Shader::setVec3 (const std::string& n, float x, float y, float z) const { glUniform3f(glGetUniformLocation(ID, n.c_str()), x, y, z); }
void Shader::setMat4 (const std::string& n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(ID, n.c_str()), 1, GL_FALSE, glm::value_ptr(m)); }
```

**Penjelasan kunci:**
- `readFile` pakai `stringstream` — cara idiomatic baca file teks penuh ke `std::string`. Bukan loop char-per-char.
- `checkCompile` dipakai untuk **3 hal**: vertex, fragment, dan program linking. Tidak duplikasi pengecekan error 3 kali.
- `glm::value_ptr(m)` — mat4 di GLM disimpan column-major (sama dengan OpenGL), tapi C++ `&m[0][0]` bisa ambigu. `value_ptr` aman.

> 📖 **Guide:** Section [1.5](./OPENGL_GUIDE.md#15-shaders--glsl) menjelaskan kenapa membungkus shader ke class itu best practice.

---

## Fase 2 — MVP Matriks & Class `Camera`

**Tujuan:** sebelum lighting, pastikan kita bisa render geometri 3D dari sudut pandang kamera yang bisa digerakkan. Ini fondasi untuk semua fase berikutnya.

### 2.1 `src/Camera.h`

```cpp
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
public:
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch;
    float MovementSpeed = 2.5f;
    float MouseSensitivity = 0.1f;
    float Zoom = 45.0f;

    Camera(glm::vec3 pos = glm::vec3(0,0,3),
           glm::vec3 up  = glm::vec3(0,1,0),
           float yaw = -90.0f, float pitch = 0.0f);

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }
    void ProcessKeyboard(CameraMovement dir, float dt);
    void ProcessMouseMovement(float xoff, float yoff, bool constrainPitch = true);
    void ProcessMouseScroll(float yoff);

private:
    void updateVectors();
};
```

### 2.2 `src/Camera.cpp`

```cpp
#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 up, float yaw, float pitch)
    : Position(pos), WorldUp(up), Yaw(yaw), Pitch(pitch) {
    updateVectors();
}

void Camera::ProcessKeyboard(CameraMovement dir, float dt) {
    float v = MovementSpeed * dt;
    if (dir == FORWARD)  Position += Front * v;
    if (dir == BACKWARD) Position -= Front * v;
    if (dir == LEFT)     Position -= Right * v;
    if (dir == RIGHT)    Position += Right * v;
}

void Camera::ProcessMouseMovement(float xoff, float yoff, bool constrainPitch) {
    Yaw   += xoff * MouseSensitivity;
    Pitch += yoff * MouseSensitivity;
    if (constrainPitch) {
        if (Pitch >  89.0f) Pitch =  89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }
    updateVectors();
}

void Camera::ProcessMouseScroll(float yoff) {
    Zoom -= yoff;
    if (Zoom < 1.0f)  Zoom = 1.0f;
    if (Zoom > 90.0f) Zoom = 90.0f;
}

void Camera::updateVectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    f.y = sin(glm::radians(Pitch));
    f.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(f);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}
```

**Penjelasan kunci:**
- **`Front = (cos(yaw)cos(pitch), sin(pitch), sin(yaw)cos(pitch))`** — konversi Euler angle (yaw, pitch) ke unit direction vector. Detail di [section 1.9](./OPENGL_GUIDE.md#19-camera).
- **Pitch clamping 89°** — di 90°, `cos(pitch) = 0` → Front jadi `(0, ±1, 0)` (lurus ke atas), tegak lurus dengan `WorldUp` → `cross` jadi nol → kamera kollaps (gimbal lock).
- **Zoom = FOV** — naik/turun field of view kasih efek seperti zoom kamera.

### 2.3 Wire-up di `main.cpp`

Edit `src/main.cpp` ganti seluruh isinya:

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "Shader.h"
#include "Camera.h"

const unsigned int SCR_W = 1280, SCR_H = 720;

// --- Camera & timing globals (untuk callback) ---
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
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

void processInput(GLFWwindow* w) {
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(w, true);
    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD,  deltaTime);
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT,     deltaTime);
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT,    deltaTime);
}

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

    // ... (Fase 3 akan isi VBO/VAO + light/material setup di sini)

    while (!glfwWindowShouldClose(window)) {
        float now = glfwGetTime();
        deltaTime = now - lastFrame; lastFrame = now;
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_W / SCR_H, 0.1f, 100.0f);
        glm::mat4 view  = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        // Rotasi pelan supaya objek terlihat 3D (bukan flat persegi).
        model = glm::rotate(model, (float)glfwGetTime() * 0.5f, glm::vec3(0.5f, 1.0f, 0.0f));

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);

        // ... (draw call ditambahkan di Fase 3)

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
```

**Penjelasan kunci:**
- **`glEnable(GL_DEPTH_TEST)`** wajib untuk 3D. Tanpa ini, segitiga belakang akan menimpa segitiga depan. Plus harus clear `GL_DEPTH_BUFFER_BIT` tiap frame.
- **`GLFW_CURSOR_DISABLED`** — sembunyikan + lock cursor di tengah window (FPS-style).
- **Path `../shaders/...`** karena binary jalan dari folder `build/`.

> 📖 **Guide:** Section [1.8 MVP](./OPENGL_GUIDE.md#18-coordinate-systems-mvp) untuk penjelasan kenapa urutan `P * V * M * pos` di shader.

---

## Fase 3 — Vertex & Fragment Shader Phong

**Tujuan:** implementasi model Phong lengkap (Ambient + Diffuse + Specular). Kita gunakan kubus dulu sebagai placeholder; model `.obj` masuk di Fase 5.

### 3.1 `shaders/phong.vert`

```glsl
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal  = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

**Penjelasan kunci:**
- **`FragPos = vec3(model * vec4(aPos, 1.0))`** — posisi vertex di **world space** (bukan local). Lighting butuh world-space karena light position juga di world space.
- **`mat3(transpose(inverse(model)))`** — disebut **normal matrix**. Kalau model di-scale non-uniform (misal `scale(2,1,1)`), normal jadi bengkok kalau cuma dikali `model` biasa. Trik ini mengoreksi. Penjelasan lengkap di [section 2.2](./OPENGL_GUIDE.md#22-basic-lighting-phong-model).

### 3.2 `shaders/phong.frag`

```glsl
#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light    light;
uniform vec3     viewPos;

void main() {
    // --- Ambient ---
    vec3 ambient = light.ambient * material.ambient;

    // --- Diffuse ---
    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = light.diffuse * (diff * material.diffuse);

    // --- Specular ---
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular   = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

**Penjelasan kunci tiap komponen:**

| Komponen | Rumus | Intuisi |
|---|---|---|
| Ambient | `light.ambient × material.ambient` | "Background light" yang ada di mana-mana. Tanpa ini, sisi yang menghadap menjauhi sumber cahaya hitam total. |
| Diffuse | `dot(N, L) × light × material` | Pantulan kasar yang menyebar. Tergantung sudut antara normal dan arah cahaya — `dot` = cosinus sudut antar unit vector. Permukaan menghadap penuh → 1.0; tegak lurus → 0.0; menjauh → negatif (di-clamp ke 0). |
| Specular | `pow(dot(V, R), shininess) × light × material` | Highlight mengkilap. `R = reflect(-L, N)` = arah cahaya setelah memantul. Kalau viewDir sejajar dengan reflectDir → kilauan maksimal. `pow` dengan `shininess` besar bikin highlight tajam & kecil; kecil bikin tersebar. |

### 3.3 Tambah kubus + draw call di `main.cpp`

Sisipkan **sebelum** `while` loop di `main.cpp`:

```cpp
// Cube vertices: posisi (vec3) + normal (vec3) — 36 vertex (6 faces × 2 triangles × 3)
float cubeVertices[] = {
    // back face (z = -0.5, normal = -Z)
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    // front face (z = +0.5, normal = +Z)
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    // left face (-X)
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    // right face (+X)
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    // bottom face (-Y)
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    // top face (+Y)
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
};

unsigned int cubeVAO, cubeVBO;
glGenVertexArrays(1, &cubeVAO);
glGenBuffers(1, &cubeVBO);
glBindVertexArray(cubeVAO);
glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
// posisi (location = 0)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
// normal (location = 1)
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
```

Lalu **di dalam** while loop, setelah `shader.setMat4("model", model)`, tambahkan:

```cpp
// Light
shader.setVec3("light.position", lightPos);
shader.setVec3("light.ambient",  0.2f, 0.2f, 0.2f);
shader.setVec3("light.diffuse",  0.5f, 0.5f, 0.5f);
shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

// Material — coral (default warna learnopengl)
shader.setVec3("material.ambient",  1.0f, 0.5f, 0.31f);
shader.setVec3("material.diffuse",  1.0f, 0.5f, 0.31f);
shader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
shader.setFloat("material.shininess", 32.0f);

shader.setVec3("viewPos", camera.Position);

glBindVertexArray(cubeVAO);
glDrawArrays(GL_TRIANGLES, 0, 36);
```

**Build & test:**
```bash
cd build && make && ./app
```

Geser dengan WASD, putar pandangan dengan mouse. **Cek**: sisi yang menghadap ke `lightPos = (1.2, 1.0, 2.0)` lebih terang.

> ⚠️ **Kalau yang tampil cuma persegi (bukan kubus):** kamera melihat sisi depan kubus tegak lurus → kelihatan flat. Tambahkan rotasi di **`main.cpp`, tepat setelah `glm::mat4 model = glm::mat4(1.0f);`** di dalam render loop:
> ```cpp
> model = glm::rotate(model, (float)glfwGetTime() * 0.5f, glm::vec3(0.5f, 1.0f, 0.0f));
> ```
> Sekarang 3 sisi kubus kelihatan dan berputar pelan.

### 3.4 Eksperimen "play around" (untuk report)

> 💡 **Catatan warna:** Tutorial awalnya pakai coral/oranye, tapi warna material bebas — yang penting `material.ambient` dan `material.diffuse` di-set ke nilai yang sama (untuk look solid color). Eksperimen di bawah tidak bergantung pada warna spesifik; pakai warna apapun yang sudah kamu set.

Semua perubahan di bawah ada di **`src/main.cpp`** kecuali eksperimen #5 (yang di shader).

**Eksperimen 1 — Ubah `shininess`**
Cari baris `shader.setFloat("material.shininess", ...);` (sekitar baris 142). Coba ganti nilainya ke `8`, `32`, `128`, `256` dan bandingkan. Makin besar → highlight makin kecil & tajam (terlihat metalik); makin kecil → highlight melebar (terlihat matte).

**Eksperimen 2 — Warnai cahaya lampu**
Cari baris `shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);` (sekitar baris 135). Ganti ke warna, mis. `(1.0f, 0.2f, 0.2f)` untuk lampu merah. Objek akan tampak "diwarnai" oleh cahaya — material cyan + lampu merah → hasilnya gelap (karena cyan = R0, sedangkan lampu hanya punya R). Insight bagus untuk report: warna akhir = `lightColor × materialColor` per channel.

**Eksperimen 3 — Animasi posisi lampu**
Cari deklarasi `glm::vec3 lightPos(1.2f, 1.0f, 2.0f);` (sekitar baris 113, **di luar** while loop). Lalu **di dalam while loop, sebelum `shader.setVec3("light.position", lightPos)`**, tambahkan:
```cpp
lightPos.x = sin(glfwGetTime()) * 2.0f;
lightPos.z = cos(glfwGetTime()) * 2.0f;
```
Lampu sekarang mengorbit kubus. Bagian sisi kubus yang terang ikut bergeser.

**Eksperimen 4 — Animasi warna lampu**
Di dalam while loop, **ganti** tiga baris `shader.setVec3("light.ambient/diffuse/specular", ...)` dengan:
```cpp
glm::vec3 lc;
lc.x = sin(glfwGetTime() * 2.0f);
lc.y = sin(glfwGetTime() * 0.7f);
lc.z = sin(glfwGetTime() * 1.3f);
shader.setVec3("light.ambient",  lc * 0.2f);
shader.setVec3("light.diffuse",  lc * 0.5f);
shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
```
Lampu berganti warna pelan (disco mode). Specular sengaja dibiarkan putih agar highlight tetap jelas.

**Eksperimen 5 — Matikan komponen Phong (di shader, bukan main.cpp)**
Buka **`shaders/phong.frag`**. Cari baris `vec3 result = ambient + diffuse + specular;` (dekat akhir). Coba ganti satu per satu:
- `vec3 result = ambient;` → cuma flat color suram, tanpa shading.
- `vec3 result = ambient + diffuse;` → ada shading, tapi tanpa kilau.
- `vec3 result = ambient + specular;` → cuma highlight di tempat gelap.

Ambil screenshot 3-4 varian untuk ditampilkan di report bersebelahan. Tidak perlu rebuild C++ — shader di-load tiap kali `./app` start, jadi cukup save + jalankan ulang.

> 📖 **Guide:** Section [2.2](./OPENGL_GUIDE.md#22-basic-lighting-phong-model) detail Phong, dan [2.1](./OPENGL_GUIDE.md#21-colors) untuk konsep "lightColor × objectColor".

---

## Fase 4 — Sistem Material (Emas, Ruby, Plastik, dll.)

**Tujuan:** memenuhi requirement *"Set the objects to be off various materials"* — render beberapa kubus dengan material berbeda secara bersamaan.

Karena fragment shader sudah menerima `struct Material`, kita cukup mengubah uniform `material.*` antar draw call.

### 4.1 Buat library material

Sisipkan di `main.cpp` (di atas `main()`):

```cpp
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
```

> **Catatan `shininess`**: tabel referensi pakai range 0–1. Phong shader pakai eksponen literal (8, 32, 128). Kita kali dengan 128 saat upload.

### 4.2 Setup lampu — WAJIB putih full intensitas

> ⚠️ **Penting sebelum render material:** Nilai preset di tabel referensi (gold/silver/dll) di-tune dengan asumsi `light.ambient/diffuse/specular = (1.0, 1.0, 1.0)`. Kalau lampu dipotong (mis. diffuse = 0.5 seperti di Fase 3), warna material ikut dipotong setengah → emas terlihat coklat suram, bukan keemasan. Ubah setup lampu di render loop:
>
> ```cpp
> shader.setVec3("light.ambient",  1.0f, 1.0f, 1.0f);
> shader.setVec3("light.diffuse",  1.0f, 1.0f, 1.0f);
> shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
> ```

### 4.3 Render baris kubus dengan material berbeda

Di dalam render loop, ganti bagian draw kubus jadi:

```cpp
glBindVertexArray(cubeVAO);
for (int i = 0; i < NUM_MATERIALS; i++) {
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, glm::vec3((i - NUM_MATERIALS / 2.0f) * 1.5f, 0.0f, 0.0f));
    // Rotasi per-kubus — penting supaya highlight specular bergerak (terlihat mengkilap)
    // dan 3 sisi kubus kelihatan (bukan flat square).
    m = glm::rotate(m, (float)glfwGetTime() * 0.5f, glm::vec3(0.5f, 1.0f, 0.0f));
    shader.setMat4("model", m);

    const auto& mat = MATERIALS[i];
    shader.setVec3 ("material.ambient",   mat.ambient);
    shader.setVec3 ("material.diffuse",   mat.diffuse);
    shader.setVec3 ("material.specular",  mat.specular);
    shader.setFloat("material.shininess", mat.shininess * 128.0f);

    glDrawArrays(GL_TRIANGLES, 0, 36);
}
```

**Build & jalankan.** Anda akan melihat 6 kubus berderet — emas, perak, ruby, emerald, plastik cyan, karet hitam — di bawah satu sumber cahaya.

### 4.3 Diskusi efek shininess pada material

- **Gold/Silver/Emerald/Ruby** punya `specular` mendekati putih → highlight terang & tajam.
- **Plastic** punya `specular = 0.5` → highlight medium, dengan warna ambient/diffuse khas plastik.
- **Black rubber** `specular = 0.4` tapi `shininess = 10` (`0.078125 × 128`) → highlight **lebar & lembut** → terasa "matte".

Insight ini cocok jadi paragraf di report.

> 📖 **Guide:** Section [2.3 Materials](./OPENGL_GUIDE.md#23-materials) plus referensi tabel material klasik di link yang sama.

---

## Fase 5 — Load Model `.obj` (Class `Mesh` + `Model`)

**Tujuan:** memenuhi item #1 (download model) dan #3 (fungsi load + VBO/VAO).

### 5.0 Download model `.obj`

Pilihan model gratis:
- **Sketchfab** ([sketchfab.com](https://sketchfab.com/)) → filter Downloadable + Free + format OBJ. Cari yang **sederhana** (single object, ≤ 50k tri). Saran: `low poly tree`, `cup`, `pokemon`, `chess piece`, `vintage car`.
- **TurboSquid** → kategori Free. Filter format `.obj`.
- **Alternatif tanpa registrasi:** [Stanford 3D Repo](https://graphics.stanford.edu/data/3Dscanrep/) (Stanford Bunny, Dragon — model klasik untuk uji rendering).

Letakkan model di `models/<nama>/<nama>.obj` (Assimp akan otomatis cari texture relatif terhadap path .obj).

```bash
mkdir -p models/myobject
# extract zip dari Sketchfab ke folder ini
```

### 5.1 `src/Mesh.h`

```cpp
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO = 0;

    Mesh(std::vector<Vertex> v, std::vector<unsigned int> i);
    void Draw(const Shader& shader) const;

private:
    unsigned int VBO = 0, EBO = 0;
    void setupMesh();
};
```

### 5.2 `src/Mesh.cpp`

```cpp
#include "Mesh.h"
#include <cstddef>

Mesh::Mesh(std::vector<Vertex> v, std::vector<unsigned int> i)
    : vertices(std::move(v)), indices(std::move(i)) {
    setupMesh();
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    // location 0 — position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Position));
    // location 1 — normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Normal));
    // location 2 — texcoord (disiapkan untuk extension nanti)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::Draw(const Shader&) const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
```

**Penjelasan kunci:**
- **VBO** menyimpan **data vertex** (posisi, normal, UV) di GPU.
- **EBO** menyimpan **index** ke vertex (hemat memory — kubus 8 vertex unik, bukan 36).
- **VAO** "merekam" konfigurasi: VBO mana yang bind, layout attribute-nya bagaimana, EBO mana. Sekali setup, draw cukup `glBindVertexArray(VAO) → glDrawElements`.
- `offsetof(Vertex, Normal)` — hitung offset compile-time. Aman & maintainable; jangan hardcode `12`.

### 5.3 `src/Model.h`

```cpp
#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include "Mesh.h"
#include "Shader.h"

class Model {
public:
    Model(const std::string& path) { loadModel(path); }
    void Draw(const Shader& shader) const {
        for (const auto& m : meshes) m.Draw(shader);
    }

private:
    std::vector<Mesh> meshes;
    std::string       directory;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};
```

### 5.4 `src/Model.cpp`

```cpp
#include "Model.h"
#include <iostream>

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate     |  // konversi semua face ke triangle
        aiProcess_GenSmoothNormals|  // bikin normal kalau tidak ada
        aiProcess_FlipUVs         |  // OpenGL UV origin di kiri-bawah (kebalikan kebanyakan tool)
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
    std::cout << "Loaded model: " << path << " (" << meshes.size() << " meshes)\n";
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene*) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v{};
        v.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        v.Normal   = mesh->HasNormals()
                     ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                     : glm::vec3(0.0f, 1.0f, 0.0f);
        v.TexCoords = mesh->mTextureCoords[0]
                      ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
                      : glm::vec2(0.0f);
        vertices.push_back(v);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    return Mesh(std::move(vertices), std::move(indices));
}
```

**Penjelasan kunci:**
- **`aiProcess_Triangulate`** — banyak file `.obj` punya face quad (4 vertex). OpenGL hanya tahu triangle. Flag ini auto-split.
- **`aiProcess_GenSmoothNormals`** — kalau .obj tidak punya normal (jarang, tapi mungkin), Assimp generate sendiri.
- **`aiProcess_FlipUVs`** — UV origin di OpenGL kiri-bawah; di kebanyakan tool 3D kanan-atas. Flag ini fix tanpa Anda manual.
- **`aiProcess_JoinIdenticalVertices`** — kalau ada vertex duplikat (sering di .obj), gabung jadi 1 + update indices. Hemat memory.
- **Rekursi `processNode`** — file 3D bisa hierarchical (mis. mobil: body, 4 ban, masing-masing punya transformasi sendiri). Untuk model sederhana, biasanya cuma satu node anak.

### 5.5 Pakai di `main.cpp`

Hapus VBO/VAO kubus manual dari Fase 3 (atau biarkan untuk dibandingkan), lalu **sebelum render loop**:

```cpp
#include "Model.h"
// ...
Model myModel("../models/myobject/myobject.obj");
```

**Di dalam render loop**, ganti loop `for (i ...)` material kubus jadi:

```cpp
shader.setVec3("light.position", lightPos);
shader.setVec3("light.ambient",  0.2f, 0.2f, 0.2f);
shader.setVec3("light.diffuse",  0.5f, 0.5f, 0.5f);
shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
shader.setVec3("viewPos", camera.Position);

for (int i = 0; i < NUM_MATERIALS; i++) {
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, glm::vec3((i - NUM_MATERIALS / 2.0f) * 2.5f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.5f));  // sesuaikan dengan skala model Anda
    shader.setMat4("model", m);

    const auto& mat = MATERIALS[i];
    shader.setVec3 ("material.ambient",   mat.ambient);
    shader.setVec3 ("material.diffuse",   mat.diffuse);
    shader.setVec3 ("material.specular",  mat.specular);
    shader.setFloat("material.shininess", mat.shininess * 128.0f);

    myModel.Draw(shader);
}
```

**Hasil:** 6 instance model `.obj` Anda berderet, masing-masing dengan material berbeda, di-light dengan Phong.

> ⚠️ **Kalau model terlalu besar/kecil di layar**: ubah `glm::scale(m, glm::vec3(X))`. Untuk Stanford bunny `X = 5` cocok; untuk model Sketchfab biasanya `0.01` – `1.0` (variasi besar).

> 📖 **Guide:** Section [3.1](./OPENGL_GUIDE.md#31-assimp), [3.2](./OPENGL_GUIDE.md#32-mesh), [3.3](./OPENGL_GUIDE.md#33-model) untuk teori lengkap.

---

## Fase 6 — Tiga Konfigurasi MVP untuk Screenshot

Tugas memberi syarat: *"Have different setups for model, projection, and view matrix"*. Kita siapkan 3 mode yang bisa di-switch dengan keyboard, supaya 3 screenshot bisa diambil dari binary yang sama.

### 6.1 Tambah toggle di `main.cpp`

Di area global, tambahkan:

```cpp
enum SceneMode { MODE_DEFAULT, MODE_ORTHO, MODE_TOPDOWN };
SceneMode sceneMode = MODE_DEFAULT;
```

Update `processInput`:

```cpp
if (glfwGetKey(w, GLFW_KEY_1) == GLFW_PRESS) sceneMode = MODE_DEFAULT;
if (glfwGetKey(w, GLFW_KEY_2) == GLFW_PRESS) sceneMode = MODE_ORTHO;
if (glfwGetKey(w, GLFW_KEY_3) == GLFW_PRESS) sceneMode = MODE_TOPDOWN;
```

### 6.2 Logika tiap mode di render loop

Ganti bagian set projection/view jadi:

```cpp
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
}
shader.setMat4("projection", projection);
shader.setMat4("view", view);
```

### 6.3 Variasikan **model matrix** per-mode

Di dalam loop material, modifikasi `m` berdasarkan mode:

```cpp
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
m = glm::scale(m, glm::vec3(0.5f));
shader.setMat4("model", m);
```

**Apa yang berbeda di tiap mode:**

| Mode | Projection | View | Model |
|---|---|---|---|
| `1` Default | Perspective (FOV = camera.Zoom) | First-person, gerak WASD + mouse | Rotasi animasi per-objek |
| `2` Ortho | **Orthographic** (paralel) | Statis dari `(0,0,10)` | Statis |
| `3` Top-down | Perspective (FOV 60°) | Dari atas `(0,8,0)` menatap `(0,0,0)` | Di-rotasi 90° agar muka mengarah ke kamera |

Ketiganya menggunakan **shader + geometry sama** — hanya matriks berbeda. Inilah inti tugas no.4.

> 📖 **Guide:** Section [1.8 MVP](./OPENGL_GUIDE.md#18-coordinate-systems-mvp) tentang perspective vs ortho dan trik `lookAt`.

---

## Fase 7 — Menyiapkan Deliverables (Report + Screenshot)

### 7.1 Ambil screenshot

Cara paling cepat di Linux: **GNOME Screenshot** atau `flameshot`:
```bash
sudo apt install flameshot
flameshot gui   # area select, langsung simpan
```

Atau **screenshot in-app dengan tombol P** — bisa ditambah dengan stb_image_write (di luar scope tutorial, optional).

**Minimal set screenshot:**
1. `screenshot_phong_default.png` — semua material, mode 1 (default).
2. `screenshot_ortho.png` — mode 2 orthographic.
3. `screenshot_topdown.png` — mode 3 top-down.
4. `screenshot_one_material_gold.png` — close-up satu material (mis. emas) untuk menonjolkan specular.
5. `screenshot_shininess_compare.png` — opsional, side-by-side shininess 8 vs 256 (bisa via dua binary atau editing kode sementara).

### 7.2 Struktur report (Markdown atau Word)

Saran outline (sesuaikan formatting dosen):

```
# Laporan Tugas OpenGL — [Nama Anggota Kelompok]

## 1. Pendahuluan
- Tujuan tugas
- Lingkup implementasi
- Tools: OpenGL 4.6 Core, GLFW, GLAD, GLM, Assimp

## 2. Implementasi Vertex & Fragment Shader (Phong Model)
- Penjelasan ambient/diffuse/specular dengan rumus
- Snippet shader phong.vert dan phong.frag
- Eksperimen: variasi shininess, warna lampu, animasi posisi lampu
- Screenshot perbandingan

## 3. Sistem Material
- Struktur Material di shader
- Tabel preset (gold, silver, ruby, emerald, plastic, rubber)
- Bagaimana shininess + specular menentukan karakter material
- Screenshot 6 material berdampingan

## 4. Fungsi Load Model 3D (VBO/VAO)
- Pipeline: file .obj → Assimp → struct Vertex → Mesh (VBO/EBO/VAO)
- Snippet kelas Mesh::setupMesh dan Model::processMesh
- Penjelasan kenapa pakai VAO (state recording) + EBO (hemat memory)
- Asal model (link Sketchfab/TurboSquid + kredit author)

## 5. Konfigurasi MVP
- Tabel 3 mode (Default / Ortho / Top-down)
- Penjelasan perbedaan perspective vs ortho dengan ilustrasi
- Screenshot tiap mode

## 6. Pembagian Kerja Tim
[Anggota A: shader+material | Anggota B: model loading | Anggota C: camera+laporan]

## 7. Kendala & Pembelajaran
- Apa yang sulit: misal normal matrix, lookAt degenerate di top-down, dll.
- Pelajaran umum tentang OpenGL state machine

## 8. Cara Menjalankan
```bash
cd build && cmake .. && make
./app
# 1 = mode default, 2 = orthographic, 3 = top-down
# WASD + mouse untuk navigasi
```

## Lampiran: Source code lengkap
```

### 7.3 Checklist final sebelum submit

- [ ] Build dari `cmake --build build` tanpa error/warning serius.
- [ ] `./app` jalan dan tidak crash setelah 30 detik navigasi.
- [ ] Tombol 1/2/3 switch mode terbukti bekerja.
- [ ] 6 material kelihatan visually distinct (emas terlihat keemasan, plastik kelihatan plastik, dst.).
- [ ] Highlight specular bergerak saat kamera berpindah (artinya specular tergantung viewPos — benar).
- [ ] Screenshot rapi, resolusi ≥ 1280×720.
- [ ] Report mencantumkan **author model** + lisensi (CC-BY biasanya butuh attribution).
- [ ] File source disertakan terstruktur: `src/`, `shaders/`, `models/`, `CMakeLists.txt`, `README.md`.

---

## Appendix — Troubleshooting Cepat

| Gejala | Kemungkinan Sebab | Fix |
|---|---|---|
| Layar hitam total, kompilasi OK | Lupa `glEnable(GL_DEPTH_TEST)` atau lupa clear depth buffer | Tambahkan keduanya |
| Model muncul tapi gelap pekat | `material.ambient` terlalu kecil, atau `light.position` di dalam objek | Tinggikan `light.ambient` ke (0.3, 0.3, 0.3), pindah `lightPos` |
| Specular tidak muncul | `material.specular` (0,0,0) atau `viewPos` tidak di-update | Set specular ≥ (0.3, 0.3, 0.3); pastikan `shader.setVec3("viewPos", camera.Position)` di tiap frame |
| Specular **kelewat terang** semua | Shininess kecil → highlight terlalu lebar | Naikkan shininess ke 64 atau 128 |
| Model terlihat "wrong way" (inside-out) | Normal di-flip oleh model | Tambahkan `aiProcess_FixInfacingNormals` atau toggle `glEnable(GL_CULL_FACE)` |
| Model terlalu besar/kecil | Skala satuan model berbeda dari scene | `glm::scale(m, glm::vec3(0.01f))` atau sebaliknya |
| Assimp link error | Lupa `find_package(assimp REQUIRED)` di CMake | Cek `CMakeLists.txt` |
| `Failed to open: ../shaders/phong.vert` | CWD bukan `build/` | Jalankan `./app` dari folder `build/`, atau pakai path absolut sementara untuk debug |
| Crash saat resize window | Aspect ratio dihitung sekali di awal (di main) | Update aspect dari ukuran window setiap frame, atau di callback |
| FPS sangat rendah dengan model besar | Polygon count tinggi tapi 6× di-draw + tanpa culling | Pakai model lebih sederhana, atau `glEnable(GL_CULL_FACE); glCullFace(GL_BACK);` |

---

## Penutup

Selamat — Anda telah jalan dari segitiga 2D hijau ke scene 3D dengan model `.obj` ber-lighting Phong, 6 material berbeda, dan 3 konfigurasi MVP. Materi tutorial ini langsung memetakan ke rubrik tugas; tinggal eksekusi, ambil screenshot, dan tulis report.

> **Saran terakhir untuk kelompok:** sebelum mulai, *agree on a Git workflow* (siapa push ke `main`, siapa pakai branch). Kalau bagi peran sesuai Fase: Fase 3+4 (shader+material), Fase 5 (loading), Fase 2+6 (camera+MVP+report) — ketiganya bisa paralel setelah Fase 0–1 selesai.
