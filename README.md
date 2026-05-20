# TVG OpenGL — 3D Renderer dengan Phong Lighting

> **Tugas Mata Kuliah Teknologi Visualisasi Grafis (TVG)**
> Teknik Elektro dan Teknologi Informasi — Universitas Gadjah Mada

Proyek ini adalah implementasi renderer 3D dari nol menggunakan **OpenGL 4.6 Core Profile**, **C++17**, dan ekosistem library standar (GLFW, GLAD, GLM, Assimp). Tujuannya mendemonstrasikan konsep dasar computer graphics: **vertex/fragment shader, Phong lighting model, material preset klasik, dan beragam konfigurasi kamera/projection**.

---

## Demo Fitur

| Mode | Tombol | Deskripsi |
|---|---|---|
| **Default** | `1` | 6 model berjajar dengan material berbeda (gold, silver, ruby, emerald, plastic, rubber), kamera FPS bebas eksplor |
| **Orthographic** | `2` | Proyeksi paralel — objek jauh tidak mengecil (kayak blueprint teknik) |
| **Top-down** | `3` | Kamera bird's-eye view dari atas, model di-rotate menghadap kamera |
| **Showcase** | `4` | 1 model emas berputar di display, lampu mengorbit 360° — efek cinematic |

Highlights:
- **Phong shading** lengkap (ambient + diffuse + specular)
- **6 material preset** klasik dari tabel devernay
- **4 konfigurasi scene** untuk demo perspective/projection/view berbeda
- Load model `.glb` / `.obj` via **Assimp**
- **FPS camera** dengan WASD + mouse look + scroll-to-zoom

---

## Tech Stack

| Komponen | Versi | Fungsi |
|---|---|---|
| C++ | 17 | Bahasa utama |
| OpenGL | 4.6 Core | Graphics API |
| GLFW | 3.3+ | Window + input handling |
| GLAD | (vendored) | OpenGL function loader |
| GLM | latest | Matrix/vector math |
| Assimp | latest | 3D model loader |
| stb_image | (vendored) | Image loader untuk texture |
| CMake | 3.10+ | Build system |

---

## Step-by-Step Setup (Ubuntu)

Tested di **Ubuntu 22.04 / 24.04**. Untuk distro lain (Debian, Pop!_OS, Mint) seharusnya jalan dengan command yang sama.

### 1. Install Dependencies Sistem

Buka terminal lalu jalankan:

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    libglfw3-dev \
    libglm-dev \
    libassimp-dev \
    libgl1-mesa-dev \
    xorg-dev
```

**Penjelasan paket:**
- `build-essential` — compiler `g++`, `make`, dan tools dasar build C/C++
- `cmake` — build system generator
- `git` — untuk clone repository
- `libglfw3-dev` — header + library GLFW (windowing + input)
- `libglm-dev` — header GLM (math library, header-only)
- `libassimp-dev` — header + library Assimp (3D model loader)
- `libgl1-mesa-dev` — OpenGL runtime + header
- `xorg-dev` — dependencies X11 yang dibutuhkan GLFW

### 2. Clone Repository

```bash
git clone https://github.com/<username>/TVG.git
cd TVG
```

> Ganti `<username>` dengan owner repository di GitHub.

### 3. Cek GPU Driver (opsional tapi penting)

Pastikan GPU driver mendukung OpenGL 4.6:

```bash
glxinfo | grep "OpenGL version"
```

Kalau perintah `glxinfo` belum ada, install dulu:

```bash
sudo apt install -y mesa-utils
```

Output yang diharapkan minimal `OpenGL version string: 4.6 ...`. Kalau versi di bawah 4.6:
- **GPU NVIDIA:** install proprietary driver via `sudo ubuntu-drivers autoinstall`
- **GPU AMD/Intel:** biasanya Mesa driver bawaan sudah cukup, tapi pastikan up-to-date dengan `sudo apt upgrade`

### 4. Setup Asset Models

Model 3D ditaruh di folder `models/`. Beberapa model contoh sudah ada (`black_dragon.glb`, `magikarp-shiny-pokemon`, `toy-dinosaur`). Default di `src/main.cpp:83`:

```cpp
Model myModel("../models/black_dragon.glb");
```

Mau pakai model lain? Edit path di baris itu — Assimp support `.obj`, `.fbx`, `.glb`, `.gltf`, `.dae`, dan banyak lainnya.

### 5. Build Project

Dari root folder project (`TVG/`):

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

**Penjelasan command:**
- `mkdir -p build` — bikin folder build (kalau belum ada)
- `cd build` — pindah ke folder build (best practice: out-of-source build)
- `cmake ..` — generate Makefile dari `CMakeLists.txt` di parent dir
- `make -j$(nproc)` — compile pakai semua core CPU

Build berhasil kalau di akhir muncul:
```
[100%] Built target app
```

### 6. Jalankan

Dari folder `build/`:

```bash
./app
```

Window OpenGL akan terbuka dengan model 3D + 6 material berjajar.

---

## Kontrol

| Input | Aksi |
|---|---|
| `W` / `A` / `S` / `D` | Gerak kamera maju/kiri/mundur/kanan |
| Mouse | Look around (FPS-style) |
| Scroll wheel | Zoom (ubah FOV) |
| `1` | Mode Default — 6 material, kamera FPS bebas |
| `2` | Mode Orthographic — proyeksi paralel |
| `3` | Mode Top-down — kamera dari atas |
| `4` | Mode Showcase — 1 model emas, light orbit cinematic |
| `ESC` | Keluar |

---

## Struktur Project

```
TVG/
├── CMakeLists.txt              # Build configuration
├── README.md                   # Anda sedang membaca file ini
├── LAPORAN.md                  # Laporan tugas + penjelasan intuitif
├── PENJELASAN_KODE.md          # Penjelasan mendalam + eksperimen what-if
├── OPENGL_GUIDE.md             # Panduan tambahan OpenGL
├── TUTORIAL.md                 # Tutorial step-by-step
│
├── src/
│   ├── main.cpp                # Entry point + render loop + scene mode
│   ├── Shader.{cpp,h}          # Wrapper kompilasi shader + uniform setter
│   ├── Camera.{cpp,h}          # FPS camera (Yaw/Pitch + WASD/mouse)
│   ├── Model.{cpp,h}           # Load .glb/.obj via Assimp
│   ├── Mesh.{cpp,h}            # VAO/VBO/EBO setup + Draw
│   └── stb_image_impl.cpp      # stb_image single-translation-unit trick
│
├── shaders/
│   ├── phong.vert              # Vertex shader (transform + normal matrix)
│   └── phong.frag              # Fragment shader (Phong: ambient+diffuse+specular)
│
├── include/
│   └── stb_image.h             # Header-only image loader
│
├── glad/                       # OpenGL function loader (vendored)
│   ├── include/glad/
│   └── src/glad.c
│
├── models/                     # 3D model assets (.glb/.obj)
│   ├── black_dragon.glb        # Default model
│   ├── magikarp-shiny-pokemon/
│   └── toy-dinosaur/
│
└── build/                      # Generated by CMake (ignored)
```

---

## Troubleshooting

### "Failed to open: ../shaders/phong.vert"
Jalankan `./app` dari folder `build/`, bukan dari root. Path shader bersifat relatif terhadap working directory.

### "ASSIMP ERROR: Unable to open file"
Pastikan path model di `src/main.cpp:83` benar. Kalau pakai model baru, taruh di `models/` lalu update path.

### Build error: "GLFW/glfw3.h: No such file"
Belum install `libglfw3-dev`. Jalankan ulang step 1.

### Build error: "assimp/Importer.hpp: No such file"
Belum install `libassimp-dev`. Jalankan ulang step 1.

### Window terbuka tapi hitam total
Cek output terminal — biasanya ada error compile shader atau load model. Cek juga GPU driver minimum OpenGL 4.6 (step 3).

### FPS rendah / lag
- Tutup aplikasi GPU-heavy lain (browser, IDE)
- Cek apakah pakai integrated GPU. Untuk laptop dengan dual GPU, force pakai discrete GPU:
  ```bash
  __NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./app
  ```

### "cannot find -lassimp" saat link
Beberapa versi Ubuntu butuh `libassimp5` runtime selain `-dev`:
```bash
sudo apt install -y libassimp5
```

---

## Dokumentasi Lebih Lanjut

| File | Isi |
|---|---|
| **[LAPORAN.md](LAPORAN.md)** | Laporan tugas dengan penjelasan intuitif (banyak analogi) untuk pemula |
| **[PENJELASAN_KODE.md](PENJELASAN_KODE.md)** | Penjelasan mendalam tiap file + eksperimen "what-if" |
| **[OPENGL_GUIDE.md](OPENGL_GUIDE.md)** | Panduan teori OpenGL |
| **[TUTORIAL.md](TUTORIAL.md)** | Tutorial step-by-step membangun project dari nol |

---

## Lisensi & Kredit

- **Material preset values** — [devernay.free.fr OpenGL materials reference](http://devernay.free.fr/cours/opengl/materials.html)
- **GLAD** — generated dari [glad.dav1d.de](https://glad.dav1d.de/)
- **stb_image** — [Sean Barrett's stb library](https://github.com/nothings/stb)
- **Model assets** — credit ke creator masing-masing di folder `models/`

Dibuat untuk keperluan akademis di mata kuliah Teknologi Visualisasi Grafis, DTETI UGM.
