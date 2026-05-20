# Panduan Lengkap OpenGL untuk Pemula Total

> Panduan ini disusun berdasarkan [learnopengl.com](https://learnopengl.com/) dengan penjelasan ulang yang lebih ramah untuk orang yang **belum pernah** menyentuh computer graphics sama sekali. Setiap konsep dijelaskan dengan analogi, lalu baru masuk ke teknis dan kode.
>
> **Target versi:** OpenGL 3.3+ Core Profile (kompatibel hingga 4.6 yang Anda pakai di project).

---

## Daftar Isi

**Bagian 0 — Pengantar Sebelum Mulai**
- [0.1 Apa itu Computer Graphics?](#01-apa-itu-computer-graphics)
- [0.2 Bagaimana komputer "menggambar"?](#02-bagaimana-komputer-menggambar)
- [0.3 Tools yang akan kita pakai](#03-tools-yang-akan-kita-pakai)
- [0.4 Math Primer (vec, mat) untuk programmer non-grafika](#04-math-primer-vec-mat-untuk-programmer-non-grafika)
- [0.5 Cara Membaca Nama Fungsi OpenGL](#05-cara-membaca-nama-fungsi-opengl)

**Bagian 1 — Getting Started**
- [1.1 OpenGL: Apa, Sejarah, dan Filosofi](#11-opengl-apa-sejarah-dan-filosofi)
- [1.2 Creating a Window (GLFW & GLAD)](#12-creating-a-window-glfw--glad)
- [1.3 Hello Window](#13-hello-window)
- [1.4 Hello Triangle — Inti dari Semuanya](#14-hello-triangle--inti-dari-semuanya)
- [1.5 Shaders & GLSL](#15-shaders--glsl)
- [1.6 Textures](#16-textures)
- [1.7 Transformations](#17-transformations)
- [1.8 Coordinate Systems (MVP)](#18-coordinate-systems-mvp)
- [1.9 Camera](#19-camera)
- [1.10 Review Bagian 1](#110-review-bagian-1)

**Bagian 2 — Lighting**
- [2.1 Colors](#21-colors)
- [2.2 Basic Lighting (Phong Model)](#22-basic-lighting-phong-model)
- [2.3 Materials](#23-materials)
- [2.4 Lighting Maps](#24-lighting-maps)
- [2.5 Light Casters](#25-light-casters)
- [2.6 Multiple Lights](#26-multiple-lights)
- [2.7 Review Bagian 2](#27-review-bagian-2)

**Bagian 3 — Model Loading**
- [3.1 Assimp](#31-assimp)
- [3.2 Mesh](#32-mesh)
- [3.3 Model](#33-model)

**Appendix**
- [A. Glosarium Fungsi OpenGL & GLFW](#a-glosarium-fungsi-opengl--glfw)
- [B. Glosarium Fungsi GLM](#b-glosarium-fungsi-glm)
- [C. Cheat Sheet GLSL](#c-cheat-sheet-glsl)

---

# Bagian 0 — Pengantar Sebelum Mulai

## 0.1 Apa itu Computer Graphics?

Bayangkan layar monitor Anda. Layar itu sebenarnya cuma **kumpulan titik kecil** yang disebut **pixel**. Resolusi 1920×1080 artinya ada ~2 juta pixel. Setiap pixel hanya tahu satu hal: **warnanya apa sekarang**.

Tugas "computer graphics" sederhananya cuma: **menentukan warna setiap pixel pada saat yang tepat**, 60× per detik (atau lebih), sehingga membentuk gambar/animasi.

Pertanyaannya: **bagaimana** kita menentukan warna pixel? Ada dua pendekatan besar:

1. **CPU rendering (software rendering)** — Anda hitung satu per satu di CPU. Lambat, karena CPU diuntungkan untuk pekerjaan rumit yang dilakukan **satu-satu**.
2. **GPU rendering** — Anda kirim "instruksi" ke GPU (Graphics Processing Unit). GPU punya **ribuan core kecil** yang bisa menghitung warna **banyak pixel sekaligus**. Cepat sekali.

OpenGL adalah cara kita "ngomong" ke GPU.

## 0.2 Bagaimana komputer "menggambar"?

GPU tidak mengerti "gambar segitiga di sini" begitu saja. GPU mengerti hal-hal **sangat primitif**:

- **Titik (point)** — satu posisi di ruang
- **Garis (line)** — dua titik dihubungkan
- **Segitiga (triangle)** — tiga titik membentuk bidang

**Semua** yang Anda lihat di game 3D — karakter, mobil, hutan — sebenarnya cuma **kumpulan ribuan/jutaan segitiga**. Bola? Banyak segitiga kecil. Wajah karakter? Segitiga. Bahkan kotak: 12 segitiga (2 per sisi × 6 sisi).

> **Kenapa segitiga?** Karena segitiga selalu **flat** (datar). Tiga titik selalu membentuk satu bidang. Kalau pakai segiempat, titik ke-4 belum tentu sebidang — bikin ambigu.

Jadi pekerjaan kita di OpenGL secara umum:

1. **Definisikan titik-titik (vertex)** dari objek
2. **Kirim ke GPU**
3. **Beri tahu GPU cara mewarnainya** (lewat program kecil bernama *shader*)
4. **Suruh GPU gambar**

## 0.3 Tools yang akan kita pakai

OpenGL itu **cuma spesifikasi** — bukan library yang bisa langsung dipanggil. Kita butuh beberapa pembantu:

| Tool | Fungsi | Analogi |
|------|--------|---------|
| **OpenGL** | API untuk bicara ke GPU | Bahasa |
| **GLFW** | Bikin window, handle keyboard/mouse | Kanvas tempat menggambar |
| **GLAD** | Loader fungsi OpenGL modern | "Kamus" yang bilang ke OS: "carikan fungsi OpenGL versi terbaru di driver GPU" |
| **GLM** | Library math (vektor, matriks) | Kalkulator |
| **stb_image** | Load file gambar (PNG, JPG) | Pembuka file gambar |
| **Assimp** | Load model 3D (.obj, .fbx, dll) | Pembuka file 3D |

Project Anda sudah ada GLFW + GLAD. Nanti kita tambah GLM, stb_image, dan Assimp.

---

## 0.4 Math Primer (vec, mat) untuk programmer non-grafika

Selama belajar grafika Anda akan **berkali-kali** ketemu `vec2`, `vec3`, `vec4`, `mat3`, `mat4`. Mari kita kupas dari nol — dengan analogi yang familiar untuk programmer Python/JS/C++.

### vec — "tuple of floats"

`vecN` di GLSL/GLM cuma **kumpulan N angka float**. Itu saja. Persis seperti tuple di Python atau array di JS:

```python
# Python
position = (0.5, -0.3, 1.0)         # ≈ vec3
color    = (1.0, 0.5, 0.2, 1.0)     # ≈ vec4 (RGBA)
```

```cpp
// GLM (C++)
glm::vec3 position(0.5f, -0.3f, 1.0f);
glm::vec4 color(1.0f, 0.5f, 0.2f, 1.0f);
```

```glsl
// GLSL (shader)
vec3 position = vec3(0.5, -0.3, 1.0);
vec4 color    = vec4(1.0, 0.5, 0.2, 1.0);
```

**Kenapa pakai vec, bukan array biasa?** Karena GPU punya **instruksi khusus** untuk operasi vec — tambah/kali 3-4 angka **sekaligus** dalam 1 clock cycle. Bukan optimisasi kecil — ini perbedaan **performa 10×+**.

**Kenapa banyak nama (vec2/3/4)?** Sesuai jumlah komponen:
- `vec2` — koordinat 2D, UV tekstur (x, y)
- `vec3` — posisi/arah 3D, warna RGB (x, y, z) atau (r, g, b)
- `vec4` — posisi homogeneous, warna RGBA (x, y, z, w) atau (r, g, b, a)

### Akses Komponen

```cpp
glm::vec3 p(1.0f, 2.0f, 3.0f);
float x = p.x;        // 1.0
float y = p.y;        // 2.0
float z = p.z;        // 3.0
// atau pakai indeks: p[0], p[1], p[2]
```

Di GLSL ada bonus: **swizzling** — ambil komponen dalam urutan apapun:
```glsl
vec4 c = vec4(1.0, 2.0, 3.0, 4.0);
vec3 rgb  = c.rgb;     // (1, 2, 3) — sama dgn c.xyz
vec2 xy   = c.xy;      // (1, 2)
vec3 bgr  = c.bgr;     // (3, 2, 1) — bisa dibalik!
```

### Operasi Dasar vec — semuanya **per-komponen**

```cpp
glm::vec3 a(1.0f, 2.0f, 3.0f);
glm::vec3 b(10.0f, 20.0f, 30.0f);

a + b;       // (11, 22, 33)  — komponen ke-i ditambah komponen ke-i
a - b;       // (-9, -18, -27)
a * b;       // (10, 40, 90)  — BUKAN dot/cross, ini perkalian komponen
a * 2.0f;    // (2, 4, 6)     — kali skalar = kali semua komponen
```

**Beda dengan matematika "biasa"!** Di matematika, vector × vector tidak terdefinisi langsung. Di GLSL/GLM, `a * b` artinya per-komponen. Untuk dot product (yang hasilnya skalar) gunakan `glm::dot(a, b)`.

### Fungsi yang sering Anda lihat

| Fungsi | Arti | Kegunaan di grafika |
|--------|------|---------------------|
| `length(v)` | panjang vector (Pythagoras) | berapa jauh dari origin? |
| `normalize(v)` | bagi v dengan length-nya | jadikan panjang = 1 (arah saja) |
| `dot(a, b)` | hasilkan skalar | "seberapa searah" 2 vector |
| `cross(a, b)` | hasilkan vector tegak lurus | dapat sumbu ke-3 dari 2 vector |
| `distance(a, b)` | jarak antara 2 titik | = `length(a - b)` |
| `mix(a, b, t)` | interpolasi linear | a saat t=0, b saat t=1 |

**Contoh konkret — kenapa `normalize`?**
> Vector arah cahaya harus panjang 1, supaya hasil `dot(normal, lightDir)` tepat range [-1, 1]. Kalau lightDir panjangnya 5, hasil dot bisa 5 → lighting jadi 5× lebih terang dari seharusnya.

**Contoh konkret — kenapa `dot`?**
> `dot(A, B)` = `|A| × |B| × cos(sudut)`. Kalau keduanya normalized (panjang 1), hasilnya = `cos(sudut)`. Jadi `dot(normal, lightDir)` = "seberapa menghadap" permukaan ke cahaya: `1.0` = menghadap penuh, `0.0` = tegak lurus, `-1.0` = membelakangi.

### mat — "matriks" = grid angka dengan arti khusus

`matN` di GLSL/GLM = **matriks N×N** (kotak angka). Tapi bukan sembarang grid — ia mewakili **transformasi**.

```cpp
glm::mat4 m;   // matriks 4×4 = 16 angka float
```

**Mengapa matriks dipakai untuk transformasi?** Karena 1 matriks bisa mewakili **gabungan** translate + rotate + scale, dan **mengkali matriks ke vector = menerapkan transformasi itu**.

```
m * v   →   v' (vector yg sudah ditransformasi)
```

Bayangkan ini seperti **fungsi** di programming:

```python
# Python style:
def transform(v):
    v = scale(v, 2.0)
    v = rotate(v, 45)
    v = translate(v, (10, 0, 0))
    return v

new_pos = transform(old_pos)
```

```cpp
// Cara matriks:
glm::mat4 m = translateMat * rotateMat * scaleMat;  // gabungan
glm::vec4 new_pos = m * old_pos;
```

Matriks itu cara **packing semua transformasi jadi 1 objek** yang bisa di-apply sekali.

### Kenapa mat4 (4×4), bukan mat3?

Trik matematika: di 3D, untuk bisa **translate** dengan matriks, kita butuh dimensi tambahan. Jadi:
- Posisi 3D `(x, y, z)` → disimpan sebagai `vec4(x, y, z, 1.0)` (komponen ke-4 = `1.0` artinya "ini titik")
- Arah 3D `(x, y, z)` → disimpan sebagai `vec4(x, y, z, 0.0)` (komponen ke-4 = `0.0` artinya "ini arah, jangan ditranslate")
- Karena vec-nya 4, matriks-nya juga 4×4 → `mat4`

Anda **tidak perlu paham matematika di balik ini sekarang**. Cukup ingat:
- **Titik (posisi)**: `vec4(pos, 1.0)`
- **Arah (vektor)**: `vec4(dir, 0.0)`
- **Transformasi**: `mat4`

### Membuat matriks transformasi (GLM)

GLM punya fungsi siap pakai — Anda **tidak akan pernah menulis 16 angka matriks manual**:

```cpp
glm::mat4 m = glm::mat4(1.0f);                              // identity (no-op)
m = glm::translate(m, glm::vec3(1.0f, 0.0f, 0.0f));         // geser
m = glm::rotate(m, glm::radians(45.0f), glm::vec3(0,1,0));  // rotasi 45° sumbu Y
m = glm::scale(m, glm::vec3(2.0f));                         // besarkan 2×
```

**Identity matrix** = matriks "kosong" yang tidak mengubah apapun saat dikalikan. Mirip angka `1` untuk perkalian biasa. Kita selalu **mulai dari identity** lalu tumpuk transformasi.

> **Pelajari sekali, pakai seumur hidup.** Anda tidak perlu mengerti aljabar linear mendalam untuk grafika dasar. Cukup pahami: matriks = "fungsi transformasi" yang di-apply ke vector dengan `*`.

---

## 0.5 Cara Membaca Nama Fungsi OpenGL

Nama fungsi OpenGL **terlihat kriptik** tapi ada polanya konsisten. Sekali Anda hafal pola ini, Anda bisa "menebak" arti fungsi tanpa baca dokumentasi.

### Format Umum

```
gl<Action><Object>[Count][Type][v]
│   │       │        │      │   │
│   │       │        │      │   └─ "v" = parameter berupa pointer ke array
│   │       │        │      └──── tipe data: f (float), i (int), ui (uint), d (double)
│   │       │        └──────────── jumlah komponen: 1, 2, 3, 4
│   │       └───────────────────── object yang diaksi: Buffer, Texture, Uniform, dll
│   └───────────────────────────── kata kerja: Gen, Bind, Delete, Get, Set, dll
└───────────────────────────────── prefix wajib: semua fungsi OpenGL diawali "gl"
```

### Contoh Pembedahan

| Fungsi | Pembedahan | Artinya |
|--------|------------|---------|
| `glGenBuffers` | `gl` + `Gen` + `Buffers` | bikin (generate) object Buffer |
| `glBindBuffer` | `gl` + `Bind` + `Buffer` | pasang (bind) Buffer ke state machine |
| `glDeleteBuffers` | `gl` + `Delete` + `Buffers` | hapus Buffer |
| `glUniform3f` | `gl` + `Uniform` + `3` + `f` | set Uniform, **3** komponen, tipe **f**loat |
| `glUniform4fv` | `gl` + `Uniform` + `4` + `f` + `v` | set Uniform, **4** float, dari pointer array |
| `glUniformMatrix4fv` | `gl` + `Uniform` + `Matrix` + `4` + `f` + `v` | set Uniform Matrix 4×4 float dari pointer |
| `glVertexAttrib3f` | `gl` + `VertexAttrib` + `3` + `f` | set vertex attribute, 3 float |
| `glTexImage2D` | `gl` + `TexImage` + `2D` | upload data image 2D ke texture |

### Suffix Tipe Data

| Suffix | Tipe |
|--------|------|
| `f` | `float` |
| `i` | `int` |
| `ui` | `unsigned int` |
| `d` | `double` |
| `b` | `byte` |
| `s` | `short` |
| `v` | parameter berupa **pointer/array** (vector form) |

Contoh: `glUniform3f(loc, 1.0f, 0.5f, 0.0f)` ↔ `glUniform3fv(loc, 1, arrayPointer)` — keduanya set 3 float, beda cara passing parameter.

### Kata Kerja yang Sering Muncul

| Kata kerja | Arti |
|------------|------|
| `Gen` | Buat object baru (dapat ID) |
| `Create` | Buat object (varian untuk shader/program) |
| `Bind` | Pasang object ke state machine |
| `Delete` | Hapus object |
| `Get` | Baca state OpenGL |
| `Set` / *(tanpa awalan)* | Tulis ke state |
| `Enable` / `Disable` | Nyalakan/matikan fitur |
| `Draw` | Eksekusi rendering |
| `Clear` | Bersihkan buffer (color/depth) |

### Konstanta `GL_*`

Semua konstanta OpenGL diawali `GL_`. Pola umum:

- **Target binding**: `GL_ARRAY_BUFFER`, `GL_ELEMENT_ARRAY_BUFFER`, `GL_TEXTURE_2D`
- **Tipe**: `GL_FLOAT`, `GL_UNSIGNED_INT`, `GL_TRIANGLES`
- **Buffer**: `GL_COLOR_BUFFER_BIT`, `GL_DEPTH_BUFFER_BIT`
- **Usage hint**: `GL_STATIC_DRAW`, `GL_DYNAMIC_DRAW`
- **Status query**: `GL_COMPILE_STATUS`, `GL_LINK_STATUS`

Dengan paham pola ini, fungsi seperti `glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW)` jadi terbaca: "isi data ke buffer yang sedang di-bind sebagai ARRAY_BUFFER, dengan hint usage STATIC_DRAW".

> 📖 **Tip:** Daftar fungsi lengkap dengan deskripsi singkat ada di **Appendix A** di akhir dokumen ini. Bookmark untuk referensi cepat.

---

# Bagian 1 — Getting Started

## 1.1 OpenGL: Apa, Sejarah, dan Filosofi

### Apa itu OpenGL?

**OpenGL** = *Open Graphics Library*. Tapi awas: ini **bukan library** (bukan kumpulan kode siap pakai). OpenGL adalah **spesifikasi** — dokumen ratusan halaman yang bilang: "fungsi `glDrawArrays` harus terima parameter ini, dan harus berperilaku seperti ini."

Yang **mengimplementasikan** spesifikasi itu adalah **vendor GPU** (NVIDIA, AMD, Intel). Mereka tulis driver yang sesuai spec. Jadi waktu Anda panggil `glDrawArrays`, sebenarnya yang jalan adalah **driver GPU Anda**.

Maintainer spec-nya: **Khronos Group** (konsorsium industri).

### Immediate Mode vs Core Profile

OpenGL punya dua "gaya":

**Immediate Mode (Legacy, OpenGL 1.x–2.x)** — gaya jadul:
```cpp
glBegin(GL_TRIANGLES);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glVertex3f( 0.5f, -0.5f, 0.0f);
    glVertex3f( 0.0f,  0.5f, 0.0f);
glEnd();
```
Mudah, tapi:
- ❌ Lambat (data tidak di GPU)
- ❌ Sembunyikan apa yang GPU lakukan
- ❌ Tidak fleksibel

**Core Profile (Modern, OpenGL 3.3+)** — yang akan kita pakai:
- ✅ Cepat (data hidup di GPU)
- ✅ Anda paham apa yang terjadi
- ✅ Fleksibel — Anda kontrol setiap tahap pipeline lewat shader

Lebih sulit di awal, tapi worth it. **Wajib** untuk graphics modern.

### OpenGL adalah State Machine

Ini konsep paling penting. **OpenGL itu state machine**. Artinya: ia punya banyak "saklar" dan "setelan" internal. Anda **ubah setelan**, lalu **panggil fungsi gambar**, dan fungsi gambar akan berperilaku sesuai setelan saat itu.

**Analogi:** Bayangkan OpenGL seperti **kompor dengan banyak knob**. Knob temperatur, knob api, knob timer. Setiap kali Anda "memasak" (menggambar), kompor pakai setelan knob saat itu. Anda ubah knob → masakan berikutnya beda.

```cpp
glClearColor(1.0f, 0.0f, 0.0f, 1.0f);  // setel state: clear color = merah
glClear(GL_COLOR_BUFFER_BIT);           // pakai state untuk hapus layar (jadi merah)

glClearColor(0.0f, 0.0f, 1.0f, 1.0f);  // ganti state: clear color = biru
glClear(GL_COLOR_BUFFER_BIT);           // sekarang jadi biru
```

Tidak ada fungsi `glClearWithColor(merah)`. Anda set state dulu, baru clear.

### OpenGL Objects

Setiap "konfigurasi besar" di OpenGL dibungkus jadi **Object**. Object cuma sebuah **ID integer** yang menunjuk ke konfigurasi di driver.

Pola umumnya selalu sama:

```cpp
unsigned int objectId;
glGenSomething(1, &objectId);          // 1. Bikin object → dapat ID
glBindSomething(GL_TARGET, objectId);  // 2. "Pasang" object ke state machine
// ... konfigurasi macam-macam ...
glBindSomething(GL_TARGET, 0);         // 3. (opsional) Lepas
```

Pola **Gen → Bind → Configure → Unbind** ini akan Anda lihat ratusan kali. Familiarkan.

---

## 1.2 Creating a Window (GLFW & GLAD)

OpenGL tidak tahu apa-apa soal window. Untuk membuat window dan menerima input, kita pakai **GLFW**.

GLAD tugasnya beda: di sistem operasi, fungsi OpenGL versi modern **tidak tersedia langsung** — kita harus minta ke driver lewat `getProcAddress`. GLAD otomatisasi ini. Setelah `gladLoadGLLoader`, semua fungsi `gl*` siap dipakai.

### Setup di CMake (Anda sudah punya)

```cmake
find_package(glfw3 3.3 REQUIRED)
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

add_executable(app src/main.cpp)
target_link_libraries(app PRIVATE glad glfw dl)
```

---

## 1.3 Hello Window

Mari bikin window kosong. Saya **bedah baris-per-baris** supaya tidak ada fungsi yang "mistis":

```cpp
#include <glad/glad.h>     // PENTING: GLAD harus sebelum GLFW
#include <GLFW/glfw3.h>
#include <iostream>
```
> **`glad/glad.h`** = header yang mendefinisikan semua fungsi OpenGL modern. **Harus include duluan** karena ia mendefinisikan ulang beberapa hal yang GLFW asumsikan belum ada.

```cpp
void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h);
}
```
> Fungsi ini dipanggil otomatis oleh GLFW **setiap kali user resize window**.
> **`glViewport(x, y, width, height)`** = "OpenGL, render ke kotak ini di window". Tanpa update ini, kalau window di-resize, gambar tetap di kotak ukuran lama.

```cpp
int main() {
    glfwInit();
```
> **`glfwInit()`** = inisialisasi library GLFW. Wajib dipanggil sekali sebelum pakai fungsi GLFW lain. Return `GLFW_TRUE` kalau sukses.

```cpp
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
```
> **`glfwWindowHint(hint, value)`** = "beritahu GLFW: window berikut yang aku bikin harus punya properti ini".
> - 3 baris ini: "minta OpenGL versi 3.3, profil Core (bukan immediate mode legacy)".
> - **Catatan**: `Hint` artinya **saran sebelum** create. Setelah window dibuat, hint diabaikan.

```cpp
    GLFWwindow* window = glfwCreateWindow(800, 600, "Belajar OpenGL", nullptr, nullptr);
```
> **`glfwCreateWindow(width, height, title, monitor, share)`** = bikin window beneran.
> - `monitor = nullptr` artinya **windowed mode** (bukan fullscreen). Kalau diisi monitor, jadi fullscreen di monitor itu.
> - `share = nullptr` artinya tidak share OpenGL context dengan window lain.
> - Return pointer ke `GLFWwindow*` (handle window).

```cpp
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
```
> **`glfwMakeContextCurrent(window)`** = "semua command OpenGL berikutnya target window ini".
> Setiap window punya **OpenGL context** sendiri (kumpulan state). Kalau ada banyak window, harus switch dulu sebelum render.

```cpp
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
```
> **`gladLoadGLLoader(loader)`** = "GLAD, carikan alamat fungsi OpenGL di driver".
> Kita kasih GLAD sebuah **fungsi pencari**: `glfwGetProcAddress`. GLAD pakai fungsi itu untuk minta alamat tiap fungsi OpenGL ke driver. Hasilnya: variabel global seperti `glDrawArrays` sekarang valid.
> **Harus dipanggil setelah `glfwMakeContextCurrent`** — karena pencarian fungsi butuh context aktif.

```cpp
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
```
> **`glfwSetFramebufferSizeCallback(window, fn)`** = "kalau window di-resize, panggil fungsi ini". Daftarkan callback yang kita tulis di atas.

```cpp
    while (!glfwWindowShouldClose(window)) {
```
> **`glfwWindowShouldClose(window)`** = cek flag "user mau tutup window?" (return `GLFW_TRUE` kalau tombol X di-klik atau kita panggil `setWindowShouldClose`).

```cpp
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
```
> **`glfwGetKey(window, key)`** = baca status tombol keyboard. Return `GLFW_PRESS` atau `GLFW_RELEASE`.
> **`glfwSetWindowShouldClose(window, true)`** = "set flag close" — loop akan berhenti di iterasi berikutnya.

```cpp
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
```
> **`glClearColor(r, g, b, a)`** = "warna yang akan dipakai pas clear". Cuma SET state, tidak menggambar apa-apa.
> **`glClear(mask)`** = beneran clear. `GL_COLOR_BUFFER_BIT` = "bersihkan color buffer (isi dengan warna dari `glClearColor`)". Kalau pakai depth juga: `GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT`.

```cpp
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
```
> **`glfwSwapBuffers(window)`** = swap buffer depan/belakang. Selama 1 frame, OpenGL menggambar ke **buffer belakang** (yang tidak terlihat). Setelah selesai, swap dengan buffer depan → gambar muncul **dalam 1 frame penuh** (mencegah flicker / tearing). Ini disebut **double buffering**.
> **`glfwPollEvents()`** = proses semua event yang tertunda (mouse, keyboard, resize, close). Tanpa ini, callback tidak pernah dipanggil dan window terasa "freeze".

```cpp
    glfwTerminate();
    return 0;
}
```
> **`glfwTerminate()`** = cleanup GLFW. Hancurkan semua window, free resource. Wajib dipanggil sebelum exit.

---

## 1.4 Hello Triangle — Inti dari Semuanya

Ini section paling penting. Pahami ini → 60% OpenGL sudah di tangan.

### Graphics Pipeline

Ketika Anda kirim data vertex ke GPU, ia melewati tahapan-tahapan ini (disebut **graphics pipeline**):

```
Vertex Data (CPU)
       │
       ▼
┌──────────────────┐
│  Vertex Shader   │ ← Anda tulis (GLSL). Transformasi tiap vertex.
└──────────────────┘
       │
       ▼
┌──────────────────┐
│ Shape Assembly   │ ← Vertex dirangkai jadi primitif (segitiga, garis)
└──────────────────┘
       │
       ▼
┌──────────────────┐
│ Geometry Shader  │ ← Opsional. Bisa bikin vertex baru.
└──────────────────┘
       │
       ▼
┌──────────────────┐
│  Rasterization   │ ← Konversi primitif → fragment (calon pixel)
└──────────────────┘
       │
       ▼
┌──────────────────┐
│ Fragment Shader  │ ← Anda tulis (GLSL). Tentukan warna tiap fragment.
└──────────────────┘
       │
       ▼
┌──────────────────┐
│ Tests & Blending │ ← Depth test, blending alpha, dsb.
└──────────────────┘
       │
       ▼
   Final Pixel
```

**Tahap yang wajib Anda tulis sendiri:**
- **Vertex Shader** — dipanggil **per vertex**
- **Fragment Shader** — dipanggil **per fragment** (kira-kira per pixel yang ditutupi segitiga)

### Normalized Device Coordinates (NDC)

GPU mengerti koordinat dalam range **-1.0 sampai +1.0** untuk x, y, z. Apapun di luar range itu di-*clip* (tidak digambar).

```
        y=+1
         │
         │
 x=-1 ───┼─── x=+1
         │
         │
        y=-1
```

Vertex shader **HARUS** menghasilkan output dalam NDC (lewat `gl_Position`).

### VBO, VAO, EBO — Trio Penting

Ketiganya adalah **object** (sesuai pola Gen-Bind-Configure).

#### VBO (Vertex Buffer Object)

Tempat menyimpan **data vertex mentah** di memori GPU. Kalau tidak pakai VBO, setiap frame data dikirim ulang dari CPU → GPU lewat PCIe → sangat lambat.

```cpp
float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

unsigned int VBO;
glGenBuffers(1, &VBO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
```

**Anatomi tiap fungsi:**

> **`glGenBuffers(count, &out)`** — "GPU, bikin `count` buffer object kosong, dan kasih aku ID-nya".
> - `count = 1` → bikin 1 buffer.
> - `&VBO` → alamat untuk simpan ID-nya. Setelah ini, `VBO` berisi angka (misal `1`).
> - **Belum** alokasi memori — baru bikin ID.

> **`glBindBuffer(target, id)`** — "pasang buffer dengan ID `id` ke slot `target` di state machine".
> - `target = GL_ARRAY_BUFFER` → slot untuk **vertex data**.
> - Setelah ini, **semua operasi `GL_ARRAY_BUFFER` selanjutnya** beraksi pada `VBO`.

> **`glBufferData(target, size, data, usage)`** — "upload `size` bytes dari `data` ke buffer yang sedang di-bind di `target`".
> - `target = GL_ARRAY_BUFFER` → upload ke VBO yang barusan di-bind.
> - `sizeof(vertices)` → total byte (`9 float × 4 byte = 36` byte).
> - `vertices` → pointer ke data CPU.
> - `usage = GL_STATIC_DRAW` → hint: data jarang berubah, akan dipakai untuk gambar. GPU simpan di memori paling cepat.
>   - Hint lain: `GL_DYNAMIC_DRAW` (sering berubah), `GL_STREAM_DRAW` (sekali pakai).

#### VAO (Vertex Array Object)

VBO cuma simpan **byte mentah**. GPU tidak tahu: "byte 0–11 itu posisi, byte 12–23 itu warna?" VAO menyimpan **konfigurasi format** ini.

> **Analogi:** VBO itu file CSV mentah. VAO itu schema-nya: kolom 1 = nama (string), kolom 2 = umur (int).

```cpp
unsigned int VAO;
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(...);

glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
```

**Anatomi:**

> **`glGenVertexArrays(count, &out)`** — analog dengan `glGenBuffers`, tapi untuk VAO. Bikin object → dapat ID.

> **`glBindVertexArray(VAO)`** — pasang VAO ini sebagai "yang sedang aktif". Mulai dari sini, **semua konfigurasi vertex attribute & EBO direkam ke VAO ini**.

> **`glVertexAttribPointer(index, size, type, normalized, stride, offset)`** — beritahu OpenGL **cara membaca byte di VBO**.
> - `index = 0` → ini atribut nomor 0 (cocok dengan `layout(location = 0)` di shader).
> - `size = 3` → tiap atribut punya **3 komponen** (x, y, z).
> - `type = GL_FLOAT` → tiap komponen tipe `float` (4 byte).
> - `normalized = GL_FALSE` → jangan auto-normalize ke [0,1] / [-1,1].
> - `stride = 3 * sizeof(float)` → **jarak byte** antar vertex (= 12 byte). Kalau atribut tersusun rapat (cuma posisi), stride = ukuran satu vertex.
> - `offset = (void*)0` → atribut ini mulai di **byte ke-0** dari tiap vertex. Cast ke `void*` karena alasan historis API.

> **`glEnableVertexAttribArray(index)`** — aktifkan atribut nomor `index`. Default-nya OFF. Tanpa enable, vertex shader tidak akan terima data dari VBO untuk atribut ini.

**Penting:** VAO **harus** di-bind sebelum `glVertexAttribPointer` & `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ...)`. Karena VAO yang "merekam" konfigurasi tersebut. Nanti pas mau gambar, cukup `glBindVertexArray(VAO)` — semua setelan kembali.

Core profile **wajib** pakai VAO. Tanpa VAO, tidak ada gambar.

#### EBO (Element Buffer Object) — Index Buffer

Persegi = 2 segitiga = 6 vertex. Tapi sebenarnya cuma 4 vertex unik (sudut). EBO biarkan kita pakai **index** untuk pakai ulang vertex.

```cpp
float vertices[] = {
     0.5f,  0.5f, 0.0f,   // 0: kanan atas
     0.5f, -0.5f, 0.0f,   // 1: kanan bawah
    -0.5f, -0.5f, 0.0f,   // 2: kiri bawah
    -0.5f,  0.5f, 0.0f    // 3: kiri atas
};
unsigned int indices[] = {
    0, 1, 3,   // segitiga 1
    1, 2, 3    // segitiga 2
};

unsigned int EBO;
glGenBuffers(1, &EBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
```

Lalu gambar pakai `glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0)` bukan `glDrawArrays`.

### Shader Minimal

**Vertex shader** (`#version 330 core` minimum untuk modern OpenGL):

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;   // atribut 0 (cocok dgn glVertexAttribPointer)

void main() {
    gl_Position = vec4(aPos, 1.0);   // langsung ke NDC (belum transformasi)
}
```

**Fragment shader:**

```glsl
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);   // RGBA: orange
}
```

### Kompilasi Shader

Shader ditulis dalam **string** GLSL, di-compile **saat runtime** oleh driver GPU (bukan saat C++ compile). Alurnya: **create → source → compile → check**.

```cpp
unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(vs, 1, &vertexSource, nullptr);
glCompileShader(vs);

int success; char log[512];
glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
if (!success) {
    glGetShaderInfoLog(vs, 512, nullptr, log);
    std::cerr << "Vertex shader error:\n" << log << std::endl;
}
```

**Anatomi:**

> **`glCreateShader(type)`** — bikin shader object kosong. Return ID (unsigned int).
> - `type = GL_VERTEX_SHADER` atau `GL_FRAGMENT_SHADER` (atau geometry/compute).
> - **Catatan**: ini `Create`, bukan `Gen` — bedanya: `Create` langsung return ID (tidak pakai out param). Kebanyakan object pakai `Gen`, tapi shader/program pakai `Create`. Ini quirk historis API.

> **`glShaderSource(shader, count, &source, length)`** — kasih string source code ke shader object.
> - `shader` → ID dari `glCreateShader`.
> - `count = 1` → kasih 1 string saja (bisa kasih array beberapa string yang disatukan).
> - `&vertexSource` → alamat pointer ke string. Tipenya `const char**` (pointer ke pointer), makanya pakai `&`.
> - `length = nullptr` → null-terminated string, ukurannya hitung otomatis.

> **`glCompileShader(shader)`** — compile! Driver parse GLSL, validate syntax, generate machine code GPU. **Tidak return apa-apa** — kita harus cek manual apakah berhasil.

> **`glGetShaderiv(shader, pname, &result)`** — baca **status** dari shader object.
> - `iv` di nama = "**i**nteger **v**ector" (hasilnya integer, ditulis ke pointer).
> - `pname = GL_COMPILE_STATUS` → "apakah compile berhasil?". Hasilnya: 1 (sukses) atau 0 (gagal).

> **`glGetShaderInfoLog(shader, maxLength, &actualLength, buffer)`** — ambil **pesan error compile**.
> - Tulis pesan ke `log` (max 512 char).
> - `actualLength = nullptr` → tidak peduli berapa panjang sebenarnya.

**Kenapa tahap "create vs source vs compile" dipisah?** Supaya bisa pakai ulang shader object dengan source berbeda, atau compile shader yang sama beberapa kali dengan tweaking.

Sama untuk fragment shader. Lalu **link** keduanya jadi **program**:

```cpp
unsigned int program = glCreateProgram();
glAttachShader(program, vs);
glAttachShader(program, fs);
glLinkProgram(program);

glGetProgramiv(program, GL_LINK_STATUS, &success);
if (!success) {
    glGetProgramInfoLog(program, 512, nullptr, log);
    std::cerr << "Link error:\n" << log << std::endl;
}

glDeleteShader(vs);
glDeleteShader(fs);
```

**Anatomi:**

> **`glCreateProgram()`** — bikin **shader program** kosong. Program = container yang menyatukan vertex + fragment shader (dan opsional geometry/tess).

> **`glAttachShader(program, shader)`** — "lampirkan" shader ke program. Bisa attach beberapa shader (1 vertex + 1 fragment minimum).

> **`glLinkProgram(program)`** — link semua shader yang di-attach jadi satu **executable**. Mirip linker di C++: cek apakah output vertex shader cocok dengan input fragment shader, dst.

> **`glGetProgramiv` / `glGetProgramInfoLog`** — sama seperti shader, tapi untuk program (cek `GL_LINK_STATUS`).

> **`glDeleteShader(shader)`** — hapus shader object. Setelah di-link ke program, shader object **tidak dibutuhkan lagi** — program sudah punya kopi-nya. Best practice: langsung delete agar tidak leak memory di GPU.

### Render Loop

```cpp
while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

**Anatomi:**

> **`glUseProgram(program)`** — "shader program yang dipakai untuk draw call selanjutnya adalah ini". Mengubah state machine. Setelah ini, semua `glDrawArrays`/`glDrawElements` pakai shader ini sampai `glUseProgram` dipanggil lagi.

> **`glBindVertexArray(VAO)`** — aktifkan VAO. Semua setelan vertex attribute yang dulu kita rekam ke VAO ini, **otomatis aktif** kembali. Tidak perlu panggil `glVertexAttribPointer` lagi.

> **`glDrawArrays(mode, first, count)`** — **EKSEKUSI** rendering!
> - `mode = GL_TRIANGLES` → rangkai vertex sebagai segitiga (setiap 3 vertex = 1 segitiga).
>   - Mode lain: `GL_LINES`, `GL_POINTS`, `GL_TRIANGLE_STRIP`, dst.
> - `first = 0` → mulai dari vertex indeks 0 di VBO.
> - `count = 3` → gambar 3 vertex (= 1 segitiga).
>
> Inilah yang **memicu pipeline**: GPU jalankan vertex shader 3× (sekali per vertex), assembly jadi triangle, rasterize, jalankan fragment shader per pixel yang ditutupi.

> Untuk EBO pakai **`glDrawElements(mode, count, type, indices)`** — sama saja tapi baca indeks dari EBO.

> **File `src/main.cpp` Anda sudah implement ini dengan baik!** Coba run, harusnya segitiga hijau muncul.

### Wireframe Mode (debug)

Mau lihat segitiga sebagai garis saja? `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)`. Balik ke normal: `GL_FILL`.

---

## 1.5 Shaders & GLSL

GLSL = **OpenGL Shading Language**. Mirip C dengan tambahan tipe vector/matrix.

### Tipe Dasar GLSL

| Tipe | Contoh |
|------|--------|
| `float`, `int`, `bool`, `uint` | scalar |
| `vec2`, `vec3`, `vec4` | vector of floats |
| `ivec2/3/4`, `bvec2/3/4` | int/bool vector |
| `mat2`, `mat3`, `mat4` | matrix (kolom-major) |
| `sampler2D`, `samplerCube` | tekstur |

### Swizzling — fitur favorit

```glsl
vec4 v = vec4(1.0, 2.0, 3.0, 4.0);
vec2 a = v.xy;       // (1, 2)
vec3 b = v.zyx;      // (3, 2, 1)
vec4 c = v.xxyy;     // (1, 1, 2, 2)
```

Bisa pakai `.xyzw`, `.rgba`, atau `.stpq` — semuanya alias yang sama.

### Input/Output antar Shader

Vertex shader bisa **kirim** data ke fragment shader pakai `out`/`in`:

**Vertex:**
```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;   // kirim ke fragment shader

void main() {
    gl_Position = vec4(aPos, 1.0);
    vertexColor = aColor;
}
```

**Fragment:**
```glsl
#version 330 core
in vec3 vertexColor;    // terima dari vertex shader (sudah di-interpolasi)
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
```

**Penting:** Nilai antar vertex **otomatis di-interpolasi** linear di rasterizer. Jadi kalau vertex A merah, vertex B biru, fragment di tengah akan ungu. Ini namanya **Gouraud shading** (atau lebih umum: barycentric interpolation).

### Uniforms — Variabel Global per-Frame

`uniform` = variabel yang nilainya **sama untuk semua vertex/fragment dalam satu draw call**. Anda set dari CPU.

**Shader:**
```glsl
#version 330 core
out vec4 FragColor;
uniform vec4 ourColor;

void main() {
    FragColor = ourColor;
}
```

**CPU:**
```cpp
glUseProgram(program);  // ← HARUS pakai program dulu
int loc = glGetUniformLocation(program, "ourColor");
float time = glfwGetTime();
float green = (sin(time) / 2.0f) + 0.5f;
glUniform4f(loc, 0.0f, green, 0.0f, 1.0f);
```

> **Tip:** Setiap tipe punya fungsi sendiri: `glUniform1f`, `glUniform3fv`, `glUniformMatrix4fv`, dst.

### Vertex Attributes Berganda

Vertex bisa punya **banyak atribut** (posisi, warna, normal, UV, dsb). Tinggal interleave datanya:

```cpp
float vertices[] = {
    // posisi          // warna
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
};

// posisi: location 0, 3 float, stride 6 float, offset 0
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
// warna: location 1, 3 float, stride 6 float, offset 3 float
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
```

### Best Practice: Bungkus Shader jadi Class

Setelah project Anda berkembang, bikin class `Shader` yang load dari file:

```cpp
class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use() { glUseProgram(ID); }
    void setBool (const std::string& name, bool v)   const;
    void setInt  (const std::string& name, int v)    const;
    void setFloat(const std::string& name, float v)  const;
    void setVec3 (const std::string& name, const glm::vec3& v) const;
    void setMat4 (const std::string& name, const glm::mat4& m) const;
};
```

Ini bikin kode bersih dan reusable.

---

## 1.6 Textures

Tekstur = gambar 2D yang "ditempel" ke permukaan segitiga. Tanpa tekstur, semua serba warna polos.

### UV / Texture Coordinates

Setiap vertex punya **koordinat tekstur** (`vec2`, biasa dinamai `aTexCoord` atau UV). Range `0.0–1.0`:

```
(0,1) ─── (1,1)
  │   gambar  │
  │           │
(0,0) ─── (1,0)
```

Pojok kiri-bawah image = `(0, 0)`. Pojok kanan-atas = `(1, 1)`.

### Load Image dengan stb_image

stb_image = single-header library, tinggal copy file `stb_image.h`:

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int width, height, nrChannels;
unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
```

### Bikin Texture Object

```cpp
unsigned int texture;
glGenTextures(1, &texture);
glBindTexture(GL_TEXTURE_2D, texture);

// Wrapping: apa yg terjadi kalau UV di luar [0,1]?
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

// Filtering: cara sampling kalau pixel layar tidak align ke texel
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// Upload data
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
             GL_RGB, GL_UNSIGNED_BYTE, data);
glGenerateMipmap(GL_TEXTURE_2D);

stbi_image_free(data);
```

### Wrapping Modes

| Mode | Efek di luar [0,1] |
|------|--------------------|
| `GL_REPEAT` | Ulangi tekstur |
| `GL_MIRRORED_REPEAT` | Ulangi tapi mirror |
| `GL_CLAMP_TO_EDGE` | Pakai warna pinggir |
| `GL_CLAMP_TO_BORDER` | Pakai warna border kustom |

### Filtering

- **`GL_NEAREST`** — ambil texel terdekat. Hasil **pixelated** / blocky. Cocok untuk pixel art.
- **`GL_LINEAR`** — interpolasi 4 texel sekitar. Hasil **smooth**.

### Mipmap

Kalau objek jauh, tekstur full-res ter-down-sampled jelek (aliasing). **Mipmap** = tekstur disimpan dalam berbagai ukuran (½, ¼, ⅛, …). GPU pilih level sesuai jarak. `glGenerateMipmap(GL_TEXTURE_2D)` bikin otomatis.

### Pakai di Shader

**Vertex:**
```glsl
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
```

**Fragment:**
```glsl
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ourTexture;

void main() {
    FragColor = texture(ourTexture, TexCoord);
}
```

### Multi-Texture (Texture Units)

Kalau mau pakai 2 tekstur sekaligus:

```cpp
glActiveTexture(GL_TEXTURE0);            // pilih unit 0
glBindTexture(GL_TEXTURE_2D, texture1);
glActiveTexture(GL_TEXTURE1);            // pilih unit 1
glBindTexture(GL_TEXTURE_2D, texture2);

glUseProgram(program);
glUniform1i(glGetUniformLocation(program, "tex1"), 0);  // tex1 → unit 0
glUniform1i(glGetUniformLocation(program, "tex2"), 1);  // tex2 → unit 1
```

**Fragment:**
```glsl
uniform sampler2D tex1;
uniform sampler2D tex2;

void main() {
    FragColor = mix(texture(tex1, TexCoord),
                    texture(tex2, TexCoord), 0.2);
}
```

> **Tip umum:** Kalau pakai stb_image untuk PNG yang ter-flip vertikal, panggil `stbi_set_flip_vertically_on_load(true)` sebelum load. OpenGL ekspektasi origin (0,0) di bawah, image format umumnya di atas.

---

## 1.7 Transformations

Untuk pindah/rotate/scale objek, kita pakai **matriks** (4×4). Gunakan library **GLM**.

```cpp
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
```

### Vektor 101

`vec3(x, y, z)` = posisi atau arah di 3D. Dasar:
- **Penjumlahan**: gerakkan
- **Perkalian skalar**: skala panjang
- **Dot product**: ukur "seberapa searah"
- **Cross product**: cari vektor tegak lurus

### Matriks 4×4 — Kenapa 4, bukan 3?

3D pakai **homogeneous coordinates** dengan komponen ke-4 (`w`). Kenapa?
- **Translasi tidak bisa dilakukan matriks 3×3.** Matriks 4×4 bisa.
- `w = 1.0` → titik. `w = 0.0` → arah (tidak terpengaruh translasi).

### Tipe Transformasi

```cpp
glm::mat4 trans = glm::mat4(1.0f);  // identity

// Translasi
trans = glm::translate(trans, glm::vec3(0.5f, -0.3f, 0.0f));

// Rotasi: sudut (radian), sumbu rotasi
trans = glm::rotate(trans, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

// Skala
trans = glm::scale(trans, glm::vec3(0.5f, 0.5f, 0.5f));
```

**PENTING — urutan!** Matriks **tidak komutatif**. `A*B ≠ B*A`. GLM apply transformasi dalam urutan **kebalikan** dari penulisan:

```cpp
mat4 m = translate(...) * rotate(...) * scale(...);
// Diterapkan: scale → rotate → translate
```

Aturan umum: **scale dulu, lalu rotate, lalu translate** (S-R-T) — biar rotate-nya di tempat dan translate-nya di posisi final.

### Kirim Matriks ke Shader

```cpp
unsigned int loc = glGetUniformLocation(program, "transform");
glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(trans));
```

**Shader:**
```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(aPos, 1.0);
}
```

---

## 1.8 Coordinate Systems (MVP)

Inilah inti grafika 3D: bagaimana koordinat objek (misal: vertex kursi di model Blender) sampai jadi pixel di layar Anda.

### 5 Ruang Koordinat

```
Local Space   →   World Space   →   View Space   →   Clip Space   →   Screen Space
            (Model)         (View)            (Projection)    (perspective divide
                                                                + viewport transform)
```

1. **Local space** — koordinat asli objek (dari Blender)
2. **World space** — objek ditempatkan di scene
3. **View space** — semuanya dilihat dari sudut pandang kamera
4. **Clip space** — sudah diproyeksikan, dalam range NDC [-1, 1]
5. **Screen space** — koordinat pixel akhir

### Matriks M, V, P

**Model Matrix (M)** — Local → World. Posisi objek di dunia:
```cpp
glm::mat4 model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
model = glm::rotate(model, glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
```

**View Matrix (V)** — World → View. Bayangkan kamera. Trik: alih-alih "geser kamera", kita **geser seluruh dunia ke arah berlawanan**:
```cpp
glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
// "Kamera mundur 3 unit" = "semua objek maju 3 unit"
```

Atau lebih elegan dengan `lookAt`:
```cpp
glm::mat4 view = glm::lookAt(
    glm::vec3(0.0f, 0.0f, 3.0f),    // posisi kamera
    glm::vec3(0.0f, 0.0f, 0.0f),    // target (yg dilihat)
    glm::vec3(0.0f, 1.0f, 0.0f)     // up vector (biasanya y atas)
);
```

**Projection Matrix (P)** — View → Clip. Dua jenis:

**Perspective** (3D normal — objek jauh terlihat kecil):
```cpp
glm::mat4 proj = glm::perspective(
    glm::radians(45.0f),         // FOV (field of view)
    (float)width / height,       // aspect ratio
    0.1f,                        // near plane
    100.0f                       // far plane
);
```

**Orthographic** (paralel — untuk 2D/CAD/UI):
```cpp
glm::mat4 proj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);
```

### Gabungan: MVP

**Shader:**
```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

**Urutan KANAN ke KIRI:** vertex → model → view → projection. Hasil akhir: clip space coordinates.

### Z-Buffer / Depth Testing

Untuk 3D, perlu **depth test** supaya objek belakang tidak nutupi objek depan:

```cpp
glEnable(GL_DEPTH_TEST);
// Di render loop:
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // ← clear depth juga!
```

Tiap fragment punya nilai z. GPU bandingkan z fragment baru dengan yang sudah di buffer → keep yang terdekat.

---

## 1.9 Camera

Kamera itu **konsep matematis** — sebenarnya tidak ada "kamera" di OpenGL. Cuma view matrix.

### Parameter Kamera Standar

```cpp
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);  // arah pandang
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
```

### Gerakan WASD

```cpp
float cameraSpeed = 2.5f * deltaTime;  // ← deltaTime penting!
if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
```

### Delta Time — Wajib!

Kecepatan gerak harus **independen** dari FPS. Tanpa delta time, di komputer cepat objek lari kencang, di komputer lambat jalan pelan.

```cpp
float deltaTime = 0.0f, lastFrame = 0.0f;

// di render loop:
float currentFrame = glfwGetTime();
deltaTime = currentFrame - lastFrame;
lastFrame = currentFrame;
```

### Mouse Look (FPS Camera)

Kita pakai **Euler angles**: **yaw** (kiri-kanan, sumbu Y) dan **pitch** (atas-bawah, sumbu X).

```cpp
float yaw = -90.0f, pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;

void mouse_callback(GLFWwindow* w, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoff = xpos - lastX;
    float yoff = lastY - ypos;  // Y terbalik
    lastX = xpos; lastY = ypos;

    const float sensitivity = 0.1f;
    xoff *= sensitivity;
    yoff *= sensitivity;

    yaw   += xoff;
    pitch += yoff;
    if (pitch >  89.0f) pitch =  89.0f;  // hindari gimbal lock
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}
```

Lalu daftarkan: `glfwSetCursorPosCallback(window, mouse_callback)` dan sembunyikan cursor `glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED)`.

### Best Practice: Bungkus jadi class

```cpp
class Camera {
public:
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch, MovementSpeed, MouseSensitivity, Zoom;

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(int direction, float dt);
    void ProcessMouseMovement(float xoff, float yoff);
    void ProcessMouseScroll(float yoff);  // zoom = ubah FOV
};
```

---

## 1.10 Review Bagian 1

Yang sudah Anda pelajari di Bagian 1:

✅ OpenGL adalah **state machine + objects** (Gen-Bind-Configure-Use)
✅ Pipeline: **vertex shader → primitive assembly → rasterizer → fragment shader**
✅ **VBO** simpan data, **VAO** simpan format, **EBO** simpan index
✅ **Shader** ditulis di GLSL. Vertex per-vertex, fragment per-pixel
✅ Komunikasi shader: `in/out` (interpolasi), `uniform` (CPU-set)
✅ Tekstur: load → buat object → bind → set wrap/filter → upload → mipmap
✅ Transformasi pakai matriks 4×4. Order: **scale → rotate → translate**
✅ **MVP**: Model → World, View → Camera, Projection → Clip
✅ Kamera = view matrix. Gunakan `deltaTime` agar movement framerate-independent

Di Bagian 2 kita kasih objek **terlihat 3D** dengan **lighting**.

---

# Bagian 2 — Lighting

## 2.1 Colors

### Cara Pandang Baru tentang Warna

Sebelum lighting, warna objek ditulis langsung (`vec3(1.0, 0.5, 0.0)`). Dengan lighting, warna yang **dilihat** = **interaksi cahaya + warna objek**.

> **Analogi:** Tomat merah di siang hari = merah. Tomat sama di kamar gelap → hitam. Yang berubah bukan tomatnya — tapi **cahayanya**.

### Konvensi Sederhana

Warna terlihat = **warna cahaya × warna objek**:
```glsl
vec3 lightColor   = vec3(1.0, 1.0, 1.0);     // putih
vec3 objectColor  = vec3(1.0, 0.5, 0.31);    // oranye-coklat
vec3 result       = lightColor * objectColor; // ini yang dilihat mata
```

Kalau lampu merah (`vec3(1.0, 0.0, 0.0)`) × objek hijau (`vec3(0.0, 1.0, 0.0)`) = hitam. Karena tidak ada komponen yang dipantul.

### Setup Scene Dua Objek

Biasanya kita render **dua object**: kubus utama (yang dicahayai) + kubus kecil (representasi lampu, pakai shader berbeda yang selalu putih cerah).

```cpp
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// Untuk lampu, pakai shader sendiri yg cuma output putih
glUseProgram(lampShader);
glm::mat4 model = glm::translate(glm::mat4(1.0f), lightPos);
model = glm::scale(model, glm::vec3(0.2f));
// set MVP, draw cube
```

---

## 2.2 Basic Lighting (Phong Model)

**Phong** = model lighting paling populer untuk realtime. Bukan akurat secara fisika, tapi cukup bagus & cepat.

Phong = **Ambient + Diffuse + Specular**.

```
finalColor = (ambient + diffuse + specular) * objectColor
```

### Ambient

Pengganti "cahaya tersebar" di ruangan. Tanpa ambient, sisi gelap objek **hitam pekat** — tidak realistis.

```glsl
float ambientStrength = 0.1;
vec3 ambient = ambientStrength * lightColor;
```

### Diffuse

Cahaya yang dipantul **rata ke segala arah** dari permukaan kasar. Intensitasnya bergantung pada **sudut antara normal permukaan dan arah cahaya**.

Untuk ini perlu **normal vector** — vektor tegak lurus permukaan (per vertex).

```glsl
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);  // dari fragment ke lampu

float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;
```

**`max(dot, 0.0)`** — kalau permukaan menghadap menjauh dari cahaya, dot produk negatif → clamp ke 0 (tidak ada kontribusi).

**Tambah Normal ke Vertex:**
```cpp
float vertices[] = {
    // posisi          // normal
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
    // ... 36 vertex (cube)
};
```

**Vertex shader:**
```glsl
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;   // posisi di world space (utk diffuse)
out vec3 Normal;

uniform mat4 model, view, projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  // normal matrix
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

> **Kenapa `transpose(inverse(model))`?** Kalau model di-scale non-uniform, normal jadi bengkok. Normal matrix mengoreksi ini. Kalau yakin scaling uniform, `mat3(model)` cukup.

### Specular

Pantulan **terang & terfokus** di permukaan mengkilap. Bergantung pada arah pandang kamera.

```glsl
float specularStrength = 0.5;
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);

float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//                                                  ↑ shininess
vec3 specular = specularStrength * spec * lightColor;
```

**Shininess** (eksponen) — makin tinggi, highlight makin kecil & tajam (16, 32, 64, 128, 256).

### Output Final

```glsl
vec3 result = (ambient + diffuse + specular) * objectColor;
FragColor = vec4(result, 1.0);
```

**CPU set uniform:**
```cpp
shader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
shader.setVec3("lightColor",  1.0f, 1.0f, 1.0f);
shader.setVec3("lightPos",    lightPos);
shader.setVec3("viewPos",     camera.Position);
```

---

## 2.3 Materials

Permukaan berbeda merespon cahaya berbeda. Emas ≠ plastik ≠ karet. Kita modelkan ini dengan **struct Material** di shader.

```glsl
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;
```

Dan **Light** dipisah:
```glsl
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;
```

**Fragment shader:**
```glsl
vec3 ambient  = light.ambient  * material.ambient;
vec3 diffuse  = light.diffuse  * (diff * material.diffuse);
vec3 specular = light.specular * (spec * material.specular);
```

**Set dari CPU** (pakai class Shader yg sudah Anda buat):
```cpp
// Material kayu mas (gold)
shader.setVec3("material.ambient",   0.24f, 0.20f, 0.07f);
shader.setVec3("material.diffuse",   0.75f, 0.61f, 0.23f);
shader.setVec3("material.specular",  0.63f, 0.56f, 0.37f);
shader.setFloat("material.shininess", 51.2f);
```

> Referensi material klasik: http://devernay.free.fr/cours/opengl/materials.html

---

## 2.4 Lighting Maps

Material polos = setiap titik di kubus warnanya sama. Tidak realistis. **Lighting map** = pakai **tekstur** untuk variasikan properti material per-fragment.

### Diffuse Map

Diganti `material.diffuse` (vec3) dengan **`sampler2D`**:

```glsl
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
```

```glsl
vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
```

Sekarang warna diffuse berbeda per pixel — sesuai gambar tekstur.

### Specular Map

Image grayscale di mana **putih = mengkilap**, **hitam = matte**.

```glsl
vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
```

Contoh: kotak kayu dengan bingkai logam → diffuse map = gambar kayu, specular map = hitam di kayu, putih di logam. Hasil: logamnya yang berkilau.

### Emission Map (opsional)

Untuk objek yang **bercahaya sendiri** (LED, lava). Tambah komponen ke output:
```glsl
vec3 emission = vec3(texture(material.emission, TexCoords));
result = ambient + diffuse + specular + emission;
```

---

## 2.5 Light Casters

Sumber cahaya bermacam-macam. Tiga jenis utama:

### 1. Directional Light (Cahaya Matahari)

Sumber **sangat jauh**, semua sinar sejajar. Tidak ada posisi, hanya **arah**.

```glsl
struct DirLight {
    vec3 direction;
    vec3 ambient, diffuse, specular;
};

vec3 lightDir = normalize(-light.direction);
// ... sisanya sama spt Phong
```

Tidak ada attenuation (cahaya tidak melemah dengan jarak).

### 2. Point Light (Bohlam)

Punya **posisi**, memancar ke segala arah, **melemah** dengan jarak.

**Attenuation formula:**
```
F_att = 1.0 / (Kc + Kl·d + Kq·d²)
```

| Jarak | Kc  | Kl    | Kq      |
|-------|-----|-------|---------|
| 7     | 1.0 | 0.7   | 1.8     |
| 13    | 1.0 | 0.35  | 0.44    |
| 32    | 1.0 | 0.14  | 0.07    |
| 50    | 1.0 | 0.09  | 0.032   |
| 100   | 1.0 | 0.045 | 0.0075  |
| 200   | 1.0 | 0.022 | 0.0019  |

```glsl
float distance = length(light.position - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * distance +
                            light.quadratic * (distance * distance));

ambient  *= attenuation;
diffuse  *= attenuation;
specular *= attenuation;
```

### 3. Spotlight (Senter)

Punya **posisi + arah**, hanya cahayai dalam **kerucut**.

Dua sudut:
- **Inner cutoff** (`γ`) — di dalamnya intensitas penuh
- **Outer cutoff** (`Φ`) — di luarnya gelap. Antara inner-outer: fade lembut

```glsl
vec3 lightDir = normalize(light.position - FragPos);
float theta = dot(lightDir, normalize(-light.direction));
float epsilon = light.cutOff - light.outerCutOff;
float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

diffuse  *= intensity;
specular *= intensity;
```

**Penting:** `cutOff` & `outerCutOff` disimpan sebagai **cosinus** sudut (`cos(glm::radians(12.5f))`), bukan sudut langsung — karena `dot product` hasilnya cosinus.

---

## 2.6 Multiple Lights

Scene realistis = banyak sumber cahaya. Kombinasi: 1 directional (matahari) + N point lights (lampu) + 1 spotlight (senter pemain).

Bikin fungsi GLSL untuk tiap tipe:

```glsl
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
```

Lalu di `main()`:
```glsl
void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    for (int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);

    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
    FragColor = vec4(result, 1.0);
}
```

Array uniform:
```glsl
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];
```

Set per element: `shader.setVec3("pointLights[0].position", ...)`.

---

## 2.7 Review Bagian 2

✅ Warna terlihat = **interaksi cahaya × objek**
✅ **Phong** = ambient + diffuse + specular
✅ **Diffuse** butuh normal vector; **specular** butuh view direction
✅ **Material** struct + **Light** struct
✅ **Lighting maps** = tekstur untuk diffuse & specular per-fragment
✅ **Directional / Point / Spotlight** — masing-masing dengan formula khas
✅ **Multiple lights** = jumlahkan kontribusi tiap lampu dengan fungsi GLSL

Di Bagian 3, kita load **model 3D asli** (bukan cuma kubus manual).

---

# Bagian 3 — Model Loading

## 3.1 Assimp

Sampai sini, semua geometri kita ketik manual (array float). Realitas: artist 3D bikin model di Blender/Maya → export ke file (`.obj`, `.fbx`, `.gltf`, `.dae`, dll). Kita perlu **parser** untuk file-file ini.

**Assimp** (*Open Asset Import Library*) = library yang baca **puluhan format** 3D dan ekspos data dalam **struktur seragam**.

### Setup

**Ubuntu:**
```bash
sudo apt install libassimp-dev
```

**CMake:**
```cmake
find_package(assimp REQUIRED)
target_link_libraries(app PRIVATE assimp)
```

**Include:**
```cpp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
```

### Struktur Data Assimp

```
aiScene (root)
├── mRootNode (aiNode)
│    ├── mMeshes[]  (index ke aiScene->mMeshes)
│    └── mChildren[] (aiNode lain — hierarki)
├── mMeshes[]     (semua aiMesh)
└── mMaterials[]  (semua aiMaterial)

aiNode adalah hierarchical (parent-child).
aiMesh berisi: mVertices, mNormals, mTextureCoords, mFaces.
aiFace adalah primitif (biasanya triangle dengan 3 mIndices).
```

> **Analogi:** `aiScene` itu seperti folder besar. `aiNode` seperti hirarki folder. `aiMesh` adalah "bagian" yg sebenarnya bisa digambar (misal: 1 model robot bisa punya 5 mesh: kepala, badan, tangan kiri, tangan kanan, kaki).

### Load Model

```cpp
Assimp::Importer importer;
const aiScene* scene = importer.ReadFile(path,
    aiProcess_Triangulate |       // paksa semua jadi triangle
    aiProcess_FlipUVs |           // flip UV (sesuaikan dgn OpenGL)
    aiProcess_GenSmoothNormals |  // bikin normal kalau belum ada
    aiProcess_CalcTangentSpace);  // untuk normal mapping (lanjutan)

if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cerr << "Assimp: " << importer.GetErrorString() << std::endl;
    return;
}
```

Lalu **traversal rekursif** ke node tree:
```cpp
void processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}
```

---

## 3.2 Mesh

Class `Mesh` kita = abstraksi 1 buah `aiMesh` yang sudah jadi VAO+VBO+EBO siap-render.

### Struktur Data

```cpp
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;   // "texture_diffuse", "texture_specular", dst.
    std::string path;   // untuk caching
};

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;
    unsigned int VAO;

    Mesh(std::vector<Vertex> v, std::vector<unsigned int> i, std::vector<Texture> t);
    void Draw(Shader& shader);

private:
    unsigned int VBO, EBO;
    void setupMesh();
};
```

### Setup VAO/VBO/EBO

```cpp
void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    // posisi
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Normal));
    // tex coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}
```

> **Trik penting:** Karena `Vertex` adalah struct dengan layout sekuensial, kita bisa langsung `sizeof(Vertex)` sebagai stride dan `offsetof()` untuk offset. Tidak perlu hitung manual.

### Draw

```cpp
void Mesh::Draw(Shader& shader) {
    unsigned int diffuseNr = 1, specularNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name = textures[i].type;
        if      (name == "texture_diffuse")  number = std::to_string(diffuseNr++);
        else if (name == "texture_specular") number = std::to_string(specularNr++);

        shader.setInt(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}
```

Shader-nya harus punya uniform `material.texture_diffuse1`, `material.texture_specular1`, dst.

---

## 3.3 Model

Class `Model` = container untuk **banyak Mesh** + logic untuk parse `aiScene`.

```cpp
class Model {
public:
    Model(const std::string& path) { loadModel(path); }
    void Draw(Shader& shader) {
        for (auto& mesh : meshes) mesh.Draw(shader);
    }

private:
    std::vector<Mesh>    meshes;
    std::vector<Texture> textures_loaded;  // cache, hindari load 2×
    std::string directory;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat,
                                              aiTextureType type,
                                              const std::string& typeName);
};
```

### processMesh

```cpp
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // VERTICES
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        v.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        v.Normal   = { mesh->mNormals[i].x,  mesh->mNormals[i].y,  mesh->mNormals[i].z  };
        if (mesh->mTextureCoords[0])
            v.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        else
            v.TexCoords = { 0.0f, 0.0f };
        vertices.push_back(v);
    }

    // INDICES
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // MATERIALS / TEXTURES
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        auto diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE,  "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        auto specMaps    = loadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specMaps.begin(), specMaps.end());
    }

    return Mesh(vertices, indices, textures);
}
```

### loadMaterialTextures (dengan caching)

```cpp
std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat,
                                                  aiTextureType type,
                                                  const std::string& typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        // Cek cache: sudah pernah load?
        bool skip = false;
        for (auto& loaded : textures_loaded) {
            if (loaded.path == str.C_Str()) {
                textures.push_back(loaded);
                skip = true;
                break;
            }
        }
        if (!skip) {
            Texture tex;
            tex.id   = TextureFromFile(str.C_Str(), directory);
            tex.type = typeName;
            tex.path = str.C_Str();
            textures.push_back(tex);
            textures_loaded.push_back(tex);
        }
    }
    return textures;
}
```

`TextureFromFile` = helper yang panggil stb_image + bikin OpenGL texture (seperti di section Textures).

### Pakai!

```cpp
Model myModel("models/backpack/backpack.obj");

// di render loop:
shader.use();
shader.setMat4("projection", projection);
shader.setMat4("view", view);
shader.setMat4("model", glm::mat4(1.0f));
myModel.Draw(shader);
```

Selesai! Sekarang Anda bisa load model 3D apapun yang didukung Assimp.

---

## Penutup

**Selamat!** Anda sudah jalan dari pixel kosong → load model 3D ber-tekstur dengan lighting Phong. Yang dipelajari:

1. **Konsep dasar GPU & pipeline**
2. **Setup window** (GLFW + GLAD)
3. **Triangle pertama** (VBO/VAO/EBO + shader)
4. **GLSL shader programming**
5. **Tekstur**
6. **Transformasi 3D & MVP**
7. **Kamera FPS-style**
8. **Phong lighting + materials + maps**
9. **Multiple light types**
10. **Model loading** dengan Assimp

### Langkah Selanjutnya

Setelah nyaman dengan ini, lanjutkan ke **"Advanced OpenGL"** di learnopengl.com:
- Depth/stencil testing
- Blending (transparansi)
- Face culling (optimasi)
- Framebuffers (post-processing)
- Cubemaps (skybox, reflections)
- Geometry shader, Instancing, Anti-aliasing

Lalu **"Advanced Lighting"**:
- Blinn-Phong
- Gamma correction
- Shadow mapping
- Normal mapping & Parallax mapping
- HDR & Bloom
- PBR (Physically Based Rendering) — standar industri modern

### Tips Belajar

1. **Ngoding**, jangan cuma baca. Tiap konsep → coba implementasi sendiri.
2. **Pecahkan satu hal saat eror**. Jangan ubah 10 hal sekaligus.
3. **Pakai RenderDoc** untuk debug — bisa lihat state OpenGL per draw call.
4. **Aktifkan debug context** OpenGL — error langsung di-catch dengan callback.

Selamat berkarya! 🎨

---

**Sumber utama:** [learnopengl.com](https://learnopengl.com/) oleh Joey de Vries. Panduan ini adalah penjelasan ulang dengan bahasa & analogi yang lebih ramah pemula — kredit konten teknis tetap ke beliau.

---

# A. Glosarium Fungsi OpenGL & GLFW

Referensi cepat. Diurutkan per kategori.

## A.1 Lifecycle / Setup (GLFW)

| Fungsi | Tujuan |
|--------|--------|
| `glfwInit()` | Init library GLFW. Wajib pertama. |
| `glfwTerminate()` | Cleanup GLFW. Panggil sebelum exit. |
| `glfwWindowHint(hint, value)` | Set properti window yang akan dibuat berikutnya. |
| `glfwCreateWindow(w, h, title, monitor, share)` | Bikin window. Return `GLFWwindow*`. |
| `glfwMakeContextCurrent(window)` | Set window ini sebagai target command OpenGL. |
| `glfwGetProcAddress(name)` | Cari alamat fungsi OpenGL di driver. Dipakai GLAD. |
| `gladLoadGLLoader(loader)` | Load semua fungsi OpenGL ke variabel global. |
| `glfwSwapBuffers(window)` | Swap buffer depan/belakang (double buffering). |
| `glfwPollEvents()` | Proses event keyboard/mouse/resize yang tertunda. |
| `glfwWindowShouldClose(window)` | Cek apakah user mau tutup window. |
| `glfwSetWindowShouldClose(window, bool)` | Set flag close (untuk exit programmatik). |
| `glfwGetTime()` | Detik sejak init (double). |

## A.2 Input (GLFW)

| Fungsi | Tujuan |
|--------|--------|
| `glfwGetKey(window, key)` | Status tombol: `GLFW_PRESS` / `GLFW_RELEASE`. |
| `glfwSetCursorPosCallback(window, fn)` | Daftarkan callback gerakan mouse. |
| `glfwSetScrollCallback(window, fn)` | Daftarkan callback scroll. |
| `glfwSetFramebufferSizeCallback(window, fn)` | Daftarkan callback resize. |
| `glfwSetInputMode(window, mode, value)` | Setel mode input (mis. `GLFW_CURSOR` `_DISABLED`). |

## A.3 Viewport & State Global (OpenGL)

| Fungsi | Tujuan |
|--------|--------|
| `glViewport(x, y, w, h)` | Set area di window untuk rendering. |
| `glClearColor(r, g, b, a)` | Set warna yang dipakai saat clear. |
| `glClear(mask)` | Clear buffer: `GL_COLOR_BUFFER_BIT`, `GL_DEPTH_BUFFER_BIT`, dst. |
| `glEnable(cap)` | Aktifkan fitur (mis. `GL_DEPTH_TEST`, `GL_BLEND`, `GL_CULL_FACE`). |
| `glDisable(cap)` | Matikan fitur. |
| `glGetString(name)` | Ambil info: `GL_VERSION`, `GL_RENDERER`, `GL_VENDOR`. |
| `glPolygonMode(face, mode)` | Mode render: `GL_FILL` (normal) atau `GL_LINE` (wireframe). |

## A.4 Buffer Objects (VBO/EBO)

| Fungsi | Tujuan |
|--------|--------|
| `glGenBuffers(n, &ids)` | Bikin `n` buffer object. |
| `glBindBuffer(target, id)` | Aktifkan buffer di slot `target` (`GL_ARRAY_BUFFER` / `GL_ELEMENT_ARRAY_BUFFER`). |
| `glBufferData(target, size, data, usage)` | Upload `data` ke buffer aktif. `usage`: `GL_STATIC_DRAW`/`DYNAMIC_DRAW`. |
| `glBufferSubData(target, offset, size, data)` | Update sebagian data (tanpa realokasi). |
| `glDeleteBuffers(n, &ids)` | Hapus buffer. |

## A.5 Vertex Array Objects (VAO)

| Fungsi | Tujuan |
|--------|--------|
| `glGenVertexArrays(n, &ids)` | Bikin VAO. |
| `glBindVertexArray(id)` | Aktifkan VAO. Semua config attribute direkam ke VAO ini. |
| `glVertexAttribPointer(idx, size, type, norm, stride, offset)` | Definisikan format atribut ke-`idx` di VBO aktif. |
| `glEnableVertexAttribArray(idx)` | Aktifkan atribut ke-`idx`. Wajib! |
| `glDisableVertexAttribArray(idx)` | Matikan atribut. |
| `glDeleteVertexArrays(n, &ids)` | Hapus VAO. |

## A.6 Shader & Program

| Fungsi | Tujuan |
|--------|--------|
| `glCreateShader(type)` | Bikin shader object. `type`: `GL_VERTEX_SHADER` dst. |
| `glShaderSource(shader, count, &src, len)` | Pasang source code ke shader. |
| `glCompileShader(shader)` | Compile shader. |
| `glGetShaderiv(shader, pname, &out)` | Baca status (mis. `GL_COMPILE_STATUS`). |
| `glGetShaderInfoLog(shader, max, &len, buf)` | Ambil pesan error compile. |
| `glCreateProgram()` | Bikin program object. |
| `glAttachShader(program, shader)` | Lampirkan shader ke program. |
| `glLinkProgram(program)` | Link semua shader jadi executable. |
| `glGetProgramiv` / `glGetProgramInfoLog` | Sama spt shader, untuk program. |
| `glUseProgram(program)` | Aktifkan program untuk draw call berikutnya. |
| `glDeleteShader(shader)` | Hapus shader object. |
| `glDeleteProgram(program)` | Hapus program. |

## A.7 Uniforms

| Fungsi | Tujuan |
|--------|--------|
| `glGetUniformLocation(program, "name")` | Cari lokasi uniform berdasar nama. Cache hasil ini! |
| `glUniform1f/2f/3f/4f(loc, ...)` | Set uniform float (1–4 komponen). |
| `glUniform1i(loc, val)` | Set uniform int (juga untuk sampler/texture unit). |
| `glUniform3fv(loc, count, ptr)` | Set uniform dari pointer array. |
| `glUniformMatrix4fv(loc, count, transpose, ptr)` | Set uniform matriks 4×4. `transpose = GL_FALSE` untuk GLM. |

## A.8 Textures

| Fungsi | Tujuan |
|--------|--------|
| `glGenTextures(n, &ids)` | Bikin texture object. |
| `glBindTexture(target, id)` | Aktifkan texture. `target = GL_TEXTURE_2D`. |
| `glActiveTexture(unit)` | Pilih texture unit aktif (`GL_TEXTURE0`, `GL_TEXTURE1`, ...). |
| `glTexImage2D(target, level, internalFmt, w, h, border, fmt, type, data)` | Upload data image ke texture. |
| `glTexParameteri(target, pname, value)` | Setel wrap/filter (`GL_TEXTURE_WRAP_S`, `GL_TEXTURE_MIN_FILTER`, dst). |
| `glGenerateMipmap(target)` | Bikin mipmap dari level 0. |
| `glDeleteTextures(n, &ids)` | Hapus texture. |

## A.9 Drawing

| Fungsi | Tujuan |
|--------|--------|
| `glDrawArrays(mode, first, count)` | Gambar dari vertex `first`, sebanyak `count`. |
| `glDrawElements(mode, count, type, indices)` | Gambar pakai EBO. `type = GL_UNSIGNED_INT` umumnya. |
| `glDrawArraysInstanced` / `glDrawElementsInstanced` | Gambar banyak instance (untuk rumput, partikel, dst). |

> **`mode`** umum: `GL_TRIANGLES`, `GL_TRIANGLE_STRIP`, `GL_LINES`, `GL_POINTS`.

---

# B. Glosarium Fungsi GLM

GLM = library math C++ yang **meniru API GLSL** — jadi syntax `glm::vec3` mirip `vec3` di shader.

## B.1 Tipe Data

| Tipe | Isi |
|------|-----|
| `glm::vec2/3/4` | Vector float. |
| `glm::ivec2/3/4` | Vector int. |
| `glm::mat3` | Matriks 3×3. |
| `glm::mat4` | Matriks 4×4 (paling sering). |
| `glm::quat` | Quaternion (rotasi tanpa gimbal lock). |

## B.2 Konstruktor

```cpp
glm::vec3 v(1.0f);              // (1, 1, 1)
glm::vec3 v(1.0f, 2.0f, 3.0f);  // (1, 2, 3)
glm::vec4 v(vec3, 1.0f);        // gabungkan vec3 + scalar
glm::mat4 m(1.0f);              // identity matrix
```

## B.3 Operasi Vector

| Fungsi | Tujuan |
|--------|--------|
| `glm::length(v)` | Panjang vector. |
| `glm::normalize(v)` | Vector arah (panjang = 1). |
| `glm::dot(a, b)` | Dot product (skalar). |
| `glm::cross(a, b)` | Cross product (vec3 tegak lurus). |
| `glm::distance(a, b)` | Jarak antar 2 titik. |
| `glm::mix(a, b, t)` | Interpolasi linear. |
| `glm::clamp(v, lo, hi)` | Batasi nilai antara lo–hi. |
| `glm::reflect(I, N)` | Pantulan vector `I` thd normal `N`. |
| `glm::radians(deg)` | Konversi derajat → radian. |

## B.4 Transformasi Matriks

| Fungsi | Tujuan |
|--------|--------|
| `glm::translate(M, vec3)` | M × matriks translate. |
| `glm::rotate(M, radians, axis)` | M × matriks rotate. |
| `glm::scale(M, vec3)` | M × matriks scale. |
| `glm::lookAt(eye, center, up)` | Bikin view matrix kamera. |
| `glm::perspective(fovY, aspect, near, far)` | Projection perspective. |
| `glm::ortho(left, right, bottom, top, near, far)` | Projection ortho. |
| `glm::inverse(M)` | Invers matriks. |
| `glm::transpose(M)` | Transpos matriks. |

## B.5 Helper untuk OpenGL

| Fungsi | Tujuan |
|--------|--------|
| `glm::value_ptr(v atau M)` | Ambil pointer ke data raw — dipakai untuk `glUniformMatrix4fv`. |

---

# C. Cheat Sheet GLSL

GLSL = bahasa shader. Mirip C, tapi punya tipe vec/mat built-in dan banyak fungsi math.

## C.1 Tipe

```glsl
float, int, uint, bool, double
vec2, vec3, vec4         // vector float
ivec2, ivec3, ivec4      // vector int
mat2, mat3, mat4         // matriks square
mat2x3, mat4x3, dll      // matriks non-square
sampler2D, samplerCube   // tekstur
```

## C.2 Qualifier (di luar fungsi)

```glsl
in vec3 aPos;             // input dari stage sebelumnya / VBO
out vec3 vertexColor;     // output ke stage berikutnya
uniform mat4 model;       // input dari CPU (sama untuk semua vertex/fragment)
layout (location = 0) in vec3 aPos;  // bind ke attribute index spesifik
```

## C.3 Built-in Variables

| Variable | Stage | Arti |
|----------|-------|------|
| `gl_Position` | vertex (out) | Posisi vertex di clip space. **Wajib di-set**. |
| `gl_PointSize` | vertex (out) | Ukuran point (kalau `GL_POINTS`). |
| `gl_FragCoord` | fragment (in) | Posisi pixel di screen. |
| `gl_FragDepth` | fragment (out) | Depth override (jarang dipakai). |

## C.4 Built-in Functions yang Sering Dipakai

### Math dasar
```glsl
abs, sign, floor, ceil, round, fract, mod
min, max, clamp(x, lo, hi)
mix(a, b, t)             // = a*(1-t) + b*t
step(edge, x), smoothstep(e0, e1, x)
sqrt, pow, exp, log
sin, cos, tan, asin, acos, atan
radians(deg), degrees(rad)
```

### Vector & Geometri
```glsl
length(v), distance(a, b)
normalize(v)
dot(a, b), cross(a, b)
reflect(I, N), refract(I, N, eta)
```

### Matriks
```glsl
transpose(M), inverse(M), determinant(M)
matrixCompMult(A, B)     // perkalian per-komponen (BUKAN matrix mul)
```

### Tekstur
```glsl
texture(sampler, uv)              // sampling 2D — fungsi paling sering!
texture(sampler, uvw)             // sampling cubemap
textureLod(sampler, uv, lod)      // pilih mipmap level manual
texelFetch(sampler, coord, lod)   // baca texel mentah (tanpa filter)
```

### Swizzling
```glsl
vec4 v = vec4(1, 2, 3, 4);
v.xyz       // (1, 2, 3)  — pakai untuk posisi
v.rgb       // (1, 2, 3)  — pakai untuk warna
v.xy        // (1, 2)
v.wzyx      // (4, 3, 2, 1)  — bisa dibalik & diulang
```

---

## Cara Pakai Glosarium Ini

1. **Saat baca kode contoh** dan ada fungsi yang asing → cari di sini.
2. **Saat menulis kode sendiri** dan butuh fungsi (mis. "gimana ya cari panjang vector?") → scan tabel kategori yang relevan.
3. **Saat error** → cek apakah parameter yang Anda kirim sesuai signature di sini.

Print/bookmark halaman ini. Setelah 1–2 minggu coding, 80% fungsi di sini sudah hafal sendiri.
