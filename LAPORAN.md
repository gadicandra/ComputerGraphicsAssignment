# Laporan OpenGL Group Assignment (Part 1)

> Implementasi: C++ / OpenGL 4.6 core + GLAD + GLFW + GLM + Assimp + stb_image.
> Source utama: `src/main.cpp`, `shaders/phong.vert`, `shaders/phong.frag`,
> `src/Model.cpp`, `src/Mesh.cpp`, `src/Shader.cpp`, `src/Camera.cpp`.

---

## Daftar Isi
0. [Bayangan Besar — Pipeline 3D itu Apa?](#0-bayangan-besar--pipeline-3d-itu-apa)
1. [Variasi Vertex Shader](#1-variasi-vertex-shader)
2. [Variasi Fragment Shader](#2-variasi-fragment-shader)
3. [Various Materials](#3-various-materials)
4. [Different Setup Model / Projection / View](#4-different-setup-model--projection--view)

---

## 0. Bayangan Besar — Pipeline 3D itu Apa?

Sebelum masuk ke detail teknis, bayangkan dulu **proses bikin animasi 3D itu mirip dengan bikin film stop-motion di studio**:

| Istilah Teknis | Bayangkan Seperti... |
|---|---|
| **Model 3D** (file .glb/.obj) | Boneka yang sudah dibikin pematung |
| **Vertex** | Titik-titik di permukaan boneka (kayak titik pada kain bertitik-titik) |
| **Mesh** | Kulit boneka — kumpulan segitiga kecil-kecil yang ditempel jadi satu permukaan |
| **Normal** | Sebatang lidi yang ditancap tegak lurus ke kulit, menunjuk "arah hadap" permukaan |
| **Texture** | Stiker/wallpaper yang ditempel di kulit boneka |
| **Vertex shader** | Tukang panggung yang menaruh boneka di posisi dan pose yang benar |
| **Fragment shader** | Pelukis yang mewarnai setiap titik kecil di kulit boneka |
| **Light/Lampu** | Lampu sorot di studio yang menerangi adegan |
| **Camera** | Kamera filmnya |
| **Render** | Hasil jepretan kameranya — gambar 2D yang muncul di layar |
| **Frame** | Satu jepretan; 60 frame per detik = 60 jepretan per detik = animasi mulus |
| **GPU** | Studio dengan ribuan pelukis kecil yang kerja bersamaan |

### Jadi alur kasarnya:
1. Komputer baca file boneka (Model) → simpan list titik (Vertex) dan instruksi gabungkannya (Index).
2. Setiap titik dikasih ke **tukang panggung** (vertex shader) untuk ditaruh di pose & posisi yang benar.
3. Komputer "menggambar" setiap segitiga di antara titik-titik itu jadi piksel-piksel layar.
4. Setiap piksel dikasih ke **pelukis** (fragment shader) untuk diwarnai berdasarkan lampu, material, dan arah pandang kamera.
5. Hasilnya: gambar 2D di layar yang **terlihat seperti 3D**.

Itu yang kita lakukan, **60 kali per detik**, untuk setiap frame. Setiap kali kamu menggerakkan mouse, looping di atas mengulang dengan posisi kamera baru.

---

## 1. Variasi Vertex Shader

File: `shaders/phong.vert`

> **Bayangkan:** vertex shader itu seperti **tukang panggung di studio film**. Sebelum kamera memotret, dia menempatkan setiap aktor (titik di permukaan boneka) di posisi yang benar di panggung, sesuai dengan adegan saat ini. Kalau adegan berubah (objek berputar/bergerak), dia menempatkan ulang semua aktor untuk frame berikutnya.

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

### Penjelasan tiap baris (versi intuitif)

| Bagian | Tujuan Teknis | Bayangan Sehari-hari |
|---|---|---|
| `in vec3 aPos` | Posisi vertex dari VBO | Koordinat asli titik di "boneka mentah" — sebelum boneka ditaruh di mana pun |
| `in vec3 aNormal` | Normal vektor | "Lidi tegak lurus" yang menunjuk ke arah luar permukaan boneka |
| `out vec3 FragPos` | Posisi di world-space, dikirim ke fragment | "Di sini titik ini berada **setelah boneka ditaruh di studio**" — penting karena nanti kita harus hitung jarak titik ini ke lampu |
| `out vec3 Normal` | Normal di world-space | Arah "lidi" setelah boneka diputar/dimiringkan — penting biar lampu tahu sisi mana yang menghadap dia |
| `uniform mat4 model` | Matriks model | **Resep transformasi**: "geser boneka ke koordinat (5, 0, 0), putar 30°, kecilkan jadi setengah" |
| `uniform mat4 view` | Matriks view | **Posisi kamera**: "kameranya ada di mana, menghadap ke mana" |
| `uniform mat4 projection` | Matriks projection | **Lensa kamera**: wide-angle atau zoom? Perspektif normal atau flat blueprint? |
| `FragPos = vec3(model * vec4(aPos, 1.0))` | Local → world | Tarik titik dari "boneka mentah" ke posisinya di "studio nyata" |
| `mat3(transpose(inverse(model))) * aNormal` | Normal matrix | Trik matematika supaya **lidi tegak lurus tetap tegak lurus** walaupun bonekanya dipencet/digepengkan |
| `gl_Position = projection * view * vec4(FragPos, 1.0)` | World → view → clip | Ambil titik di studio → lihat dari sudut pandang kamera → tentukan di mana titik itu nongol di layar |

### Kenapa `inverse-transpose` untuk normal?

> **Bayangkan:** kamu punya bola karet dengan **paku-paku tegak lurus menancap di permukaannya**. Kalau kamu **pencet bolanya jadi gepeng (scale Y = 0.5)**, paku-paku yang tadinya tegak ke atas sekarang miring — karena karetnya melar ke samping tapi tetap di paku yang sama. Itu **salah** untuk perhitungan cahaya.
>
> Lampu butuh tahu **arah permukaan yang sebenarnya**. Trik `inverse(transpose(model))` adalah resep matematika supaya pakunya **dipaksa tetap tegak lurus** ke permukaan setelah dipencet.

### Catatan optimasi
Hitung `inverse()` di vertex shader itu **mahal komputasi** dan dilakukan per-vertex (ribuan kali per frame). Production code idealnya pre-compute `mat3 normalMatrix` di CPU lalu kirim sebagai uniform. Untuk assignment scope, versi ini lebih ekspresif/edukatif.

### Variasi yang bisa ditambahkan
- **Gouraud shading** — semua kalkulasi cahaya dilakukan di vertex shader (per titik), bukan di fragment shader (per piksel). **Bayangkan:** pelukis cuma mewarnai titik-titik sudut segitiga lalu komputer menarik garis warna gradient di antaranya. Lebih cepat tapi highlight kilauan jadi **berkotak-kotak** di model low-poly.
- **Wave/displacement** — `aPos.y += sin(time + aPos.x)` untuk efek bergelombang. **Bayangkan:** bonekanya tiba-tiba kaya jelly yang bergoyang, atau permukaannya kayak bendera tertiup angin.

---

## 2. Variasi Fragment Shader

File: `shaders/phong.frag` — implementasi **Phong lighting model** klasik (3 komponen: ambient + diffuse + specular).

> **Bayangkan:** fragment shader itu **pelukis kecil yang mewarnai satu titik piksel di layar**. Untuk setiap piksel yang masuk dalam segitiga, dia bertanya: "Lampu menyinari titik ini seberapa terang? Materialnya apa (emas? karet?)? Kamera melihat dari sudut mana?" — lalu menentukan warna akhirnya.

```glsl
void main() {
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = light.diffuse * (diff * material.diffuse);

    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular   = light.specular * (spec * material.specular);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
```

### Tiga komponen Phong — pakai bayangan sehari-hari

#### A. **Ambient** — "cahaya pinjaman"
> **Bayangkan:** kamu di kamar dengan lampu dimatikan, tapi pintu sedikit terbuka — ruangan tidak gelap total karena ada cahaya yang masuk dari celah, mantul-mantul, terus sampai ke kamu. Itu **ambient**.
>
> Tanpa ambient, sisi boneka yang **tidak menghadap lampu jadi hitam pekat** seperti foto fotografer amatir. Ambient memastikan area "bayangan" tetap punya sedikit warna.

#### B. **Diffuse** — "permukaan kertas / kain"
> **Bayangkan:** kamu pegang **kertas putih** dan senter. Kalau kertas itu **lurus menghadap senter**, paling terang. Kalau dimiringkan, redup. Kalau dibalik membelakangi, gelap. Itu **diffuse**.
>
> Inilah komponen yang **membuat bentuk 3D terlihat**. Tanpa diffuse, bola dan lingkaran tidak bisa dibedakan — semua kelihatan flat seperti stiker.
>
> Rumus `dot(N, L)` itu adalah cara matematis bertanya: "Seberapa lurus permukaan ini menghadap lampu?" Nilainya 1 kalau lurus, 0 kalau tegak lurus, negatif (di-clamp ke 0) kalau membelakangi.

#### C. **Specular** — "titik kilau di sendok"
> **Bayangkan:** kamu pegang **sendok metal** di bawah lampu. Ada satu titik super terang di sendok itu. Saat kamu **gerakkan kepala**, titik itu **berpindah-pindah** mengikuti sudut pandang kamu. Itu **specular**.
>
> Specular yang bikin emas terlihat "emas" dan plastik terlihat "plastik". Tanpa specular, semua material kelihatan kayak **karton matte yang diwarnai**.
>
> **Shininess** = seberapa "halus mengkilap" permukaannya:
> - Shininess **tinggi** (128+) → highlight **kecil & tajam** seperti cermin/sendok logam
> - Shininess **rendah** (10-) → highlight **besar & lembar** seperti karet/kulit jeruk

#### Tabel ringkas
| Komponen | Rumus | Intuisi |
|---|---|---|
| **Ambient** | `light.ambient * material.ambient` | Cahaya pinjaman supaya area gelap tidak hitam total |
| **Diffuse** | `max(dot(N, L), 0) * light.diffuse * material.diffuse` | Permukaan kertas: makin lurus ke lampu makin terang |
| **Specular** | `pow(max(dot(V, R), 0), shininess) * light.specular * material.specular` | Titik kilau yang bergerak mengikuti pandangan mata |

### Variasi yang bisa ditambahkan
- **Blinn-Phong** — ganti pendekatan specular pakai "halfway vector". **Bayangkan:** alih-alih hitung pantulan cahaya yang benar, kita cari "tengah-tengah" antara arah lampu dan arah mata. Lebih murah komputasinya, dan tidak ada cut-off mendadak di sudut tajam.
- **Toon / cel-shading** — quantize gradient jadi 3-4 level diskrit. **Bayangkan:** gradient halus diubah jadi blok-blok warna kayak anime/kartun.
- **Rim light** — kasih garis cahaya tipis di pinggir siluet boneka. **Bayangkan:** efek backlight di foto profil — orangnya kayak punya "aura" tipis di pinggir.

---

## 3. Various Materials

Definisi di `src/main.cpp:43-61`. Nilai diambil dari [tabel material klasik devernay](http://devernay.free.fr/cours/opengl/materials.html), referensi standar untuk Phong material preset sejak era OpenGL fixed-function.

> **Bayangkan:** material itu seperti **resep cat** dengan 4 bumbu:
> - **Ambient**: warna "dasar" saat tidak ada cahaya langsung
> - **Diffuse**: warna utama saat kena cahaya
> - **Specular**: warna kilauan
> - **Shininess**: seberapa "mengkilat" permukaannya
>
> Mengganti material = mengganti resep = boneka berubah jadi terlihat dari logam ke karet ke plastik, walaupun bentuknya sama.

```cpp
struct MaterialPreset {
    const char* name;
    glm::vec3 ambient, diffuse, specular;
    float shininess;  // 0..1, dikali 128 saat dikirim ke shader
};
```

### Tabel preset

| # | Nama | Ambient | Diffuse | Specular | Shininess (×128) | Karakteristik visual | Bayangkan |
|---|---|---|---|---|---|---|---|
| 0 | **gold** | (0.247, 0.200, 0.075) | (0.752, 0.606, 0.226) | (0.628, 0.556, 0.366) | 51.2 | Kuning hangat, highlight terang ke-kuningan | Cincin nikah emas asli |
| 1 | **silver** | (0.192, 0.192, 0.192) | (0.508, 0.508, 0.508) | (0.508, 0.508, 0.508) | 51.2 | Abu netral, highlight putih | Sendok perak yang baru dicuci |
| 2 | **ruby** | (0.175, 0.012, 0.012) | (0.614, 0.041, 0.041) | (0.728, 0.627, 0.627) | 76.8 | Merah dalam, highlight putih kemerahan | Batu permata merah |
| 3 | **emerald** | (0.022, 0.175, 0.022) | (0.076, 0.614, 0.076) | (0.633, 0.728, 0.633) | 76.8 | Hijau pekat, highlight tajam | Batu giok |
| 4 | **cyan_plastic** | (0.0, 0.1, 0.06) | (0.0, 0.510, 0.510) | (0.502, 0.502, 0.502) | 32.0 | Cyan, kilauan lebih lebar | Mainan plastik anak-anak |
| 5 | **black_rubber** | (0.02, 0.02, 0.02) | (0.01, 0.01, 0.01) | (0.4, 0.4, 0.4) | 10.0 | Sangat gelap, highlight lebar & redup | Ban motor / sol sepatu |

### Catatan asumsi cahaya
Material preset di atas dibuat dengan asumsi **lampunya putih dan terang penuh**. Itu sebabnya di `main.cpp` lampu kita set ke nilai tinggi:
```cpp
shader.setVec3("light.ambient",  0.4, 0.4, 0.4);
shader.setVec3("light.diffuse",  0.8, 0.8, 0.8);
shader.setVec3("light.specular", 1.0, 1.0, 1.0);
```

> **Bayangkan:** kamu beli cat dengan label "warna emas mengkilap" tapi kamu pakai di kamar dengan lampu redup berwarna merah. Catnya tidak akan kelihatan emas — kelihatan oranye gelap. Material dan lampu **harus cocok asumsinya**.

### Rendering 6 material sekaligus
Di loop render, satu boneka yang sama di-render **6 kali** dengan material berbeda, ditaruh sejajar:
```cpp
m = glm::translate(m, glm::vec3((i - NUM_MATERIALS/2.0f) * 2.5f, 0, 0));
```

> **Bayangkan:** kamu punya **satu boneka master**, lalu kamu **fotokopi 6 kali** dengan jarak 2.5 meter satu sama lain. Tiap fotokopi kamu cat dengan resep berbeda (emas, perak, ruby, emerald, plastik, karet). Hasilnya: 6 boneka berjajar untuk perbandingan visual langsung — tipe "showroom material".

---

## 4. Different Setup Model / Projection / View

Toggle scene mode via keyboard **`1` / `2` / `3` / `4`** (`main.cpp:38-41`).

> **Bayangkan:** kamu **sutradara film** dan punya boneka yang sama, tapi ingin ambil **4 jenis shot berbeda**: shot dokumenter (free camera), shot blueprint engineering (ortho), shot map dari atas (top-down), dan shot showcase galeri (showcase). Masing-masing punya setting kamera, lensa, dan pose boneka yang beda.

### Triplet penting: Model, View, Projection

Sebelum masuk ke 4 mode, pahami dulu **3 matriks utama yang selalu di-set**:

| Matriks | Tugas | Bayangkan |
|---|---|---|
| **Model** | Menempatkan & memutar boneka | "Geser boneka ke titik A, putar 30°, kecilkan jadi 0.02x" |
| **View** | Menempatkan kamera | "Aku berdiri di sini, menghadap ke arah situ" |
| **Projection** | Memilih lensa kamera | "Lensa wide 45°" atau "Lensa flat blueprint" |

> **Bayangkan trio Model/View/Projection seperti syuting film:**
> - **Model** = setting properti (boneka, meja) di set
> - **View** = mengarahkan kamera ke set
> - **Projection** = memilih lensa kamera (telephoto vs wide-angle vs flat)

---

### Setup 1 — `MODE_DEFAULT` (key `1`)

**Konfigurasi:** Perspective + free-look FPS camera + animated model rotation.

> **Bayangkan:** kamu lagi main game FPS seperti Minecraft creative mode. Mouse menggerakkan pandangan, WASD untuk jalan, scroll untuk zoom. Boneka-boneka di scene berputar otomatis pelan-pelan supaya kamu lihat semua sisinya tanpa harus muter sendiri.

| Matrix | Nilai |
|---|---|
| **Projection** | `glm::perspective(radians(camera.Zoom), aspect, 0.1f, 100.0f)` — perspective dengan FOV dinamis (scroll zoom) |
| **View** | `camera.GetViewMatrix()` — FPS camera, kontrol WASD + mouse, posisi awal `(0, 2, 15)` |
| **Model** | `translate(x_offset) * rotate(time * (i+1) * 0.3, axis(0.4, 1, 0.2))` — tiap model berputar dengan kecepatan berbeda di sumbu miring |

**Tujuan visual:** view default untuk eksplor scene bebas. Karena boneka berputar relatif ke lampu, **highlight kilauan "menyapu" permukaan** — ini cara terbaik untuk lihat efek material.

---

### Setup 2 — `MODE_ORTHO` (key `2`)

**Konfigurasi:** Orthographic projection + fixed lookAt + static model.

> **Bayangkan:** kamu lihat **blueprint atau gambar teknik mesin**. Rel kereta api yang panjang **tidak menyatu di kejauhan** — tetap lebar yang sama. Mobil di belakang **tidak terlihat lebih kecil** dari mobil di depan. Itu **orthographic**.
>
> Bandingkan dengan foto biasa (perspective) — di foto rel kereta **menyempit** menjauh, itu yang otak kita asumsikan "normal". Ortho **menghilangkan kesan kedalaman jarak**.

| Matrix | Nilai |
|---|---|
| **Projection** | `glm::ortho(-8*aspect, 8*aspect, -8, 8, 0.1, 100)` — paralel, tidak ada foreshortening |
| **View** | `lookAt(eye=(0,0,10), target=(0,0,0), up=(0,1,0))` — fixed depan |
| **Model** | `translate(x_offset)` saja — tidak ada rotasi |

**Tujuan visual:** objek terlihat **flat secara perspektif** — yang jauh tidak mengecil. Sering dipakai di:
- Technical drawing / engineering blueprint
- Game strategi (StarCraft, Clash of Clans isometric-like)
- Sprite-based 2D-feel games
- UI overlays (tombol, ikon)

Membandingkan dengan setup 1 jelas terlihat perbedaan perspective vs ortho.

---

### Setup 3 — `MODE_TOPDOWN` (key `3`)

**Konfigurasi:** Perspective dari atas + model di-rotate 90° agar wajah depan menghadap kamera.

> **Bayangkan:** kamu nonton game seperti **Pacman, Civilization, atau Google Maps** — kamera dari atas melihat ke bawah. Untuk bisa melihat "wajah depan" boneka (bukan kepalanya), bonekanya **direbahkan** dulu seperti pasien di meja operasi.

| Matrix | Nilai |
|---|---|
| **Projection** | `glm::perspective(radians(60), aspect, 0.1, 100)` — FOV fixed 60° |
| **View** | `lookAt(eye=(0, 8, 0.001), target=(0,0,0), up=(0,1,0))` — kamera dari atas |
| **Model** | `translate(x_offset) * rotate(90°, axis(1,0,0))` — direbahkan biar wajah menghadap kamera atas |

> **Kenapa `z=0.001` bukan `z=0`?** Bayangkan kamu **berdiri tegak melihat ke bawah**, lalu disuruh tunjukkan "arah depan" — bingung kan? Komputer juga bingung. `0.001` itu seperti "miringkan sedikit kepala ke depan" supaya komputer tahu "depan = arah ini".

**Tujuan visual:** kamera bird's-eye view, sering dipakai di game map editor, strategy game, atau aplikasi peta. Demonstrasi: kita bisa **mengubah view + model bersamaan** untuk komposisi shot yang berbeda.

---

### Setup 4 — `MODE_SHOWCASE` (key `4`)

**Konfigurasi:** Cinematic showcase — **1 model emas**, kamera fixed di posisi "display stand", model & lampu sama-sama berputar.

> **Bayangkan:** kamu lagi nonton **iklan jam tangan mewah Rolex** — jam taruh di alas display, kamera diam, jam berputar pelan, lampu studio juga ikut bergerak supaya **kilauan emasnya menyapu seluruh permukaan secara dramatis**. Atau seperti **3D viewer produk di Shopee/Tokopedia** untuk barang mewah.

| Matrix | Nilai |
|---|---|
| **Projection** | `glm::perspective(radians(45), aspect, 0.1, 100)` — FOV 45° (lebih sempit, cinematic) |
| **View** | `lookAt(eye=(0, 1, 4), target=(0,0,0), up=(0,1,0))` — kamera sedikit di atas & mundur |
| **Model** | `rotate(time * 0.5, axis(0,1,0)) * scale(0.25)` — yaw spin pelan di sumbu Y |
| **Light** | Orbit penuh 360° radius 3.0 dengan vertical bobbing — `(sin(t*1.2)*3, 1.5+sin(t*0.6)*0.8, cos(t*1.2)*3)` |
| **Material** | Hanya `MATERIALS[0]` = **gold** |

**Tujuan visual:** showcase satu objek emas yang berputar, dengan lampu yang juga mengorbit. Kombinasi ini bikin **highlight emas terlihat hidup dan dinamis** — mata mengasosiasikan dengan "metal mahal asli".

**Kenapa harus dua-duanya berputar?**
> **Bayangkan:** kamu di museum, ada perhiasan emas di display. Kalau perhiasan **diam** dan lampu **diam**, highlight cuma muncul di satu titik mati — kelihatan kayak **stiker kuning**. Tapi kalau salah satunya bergerak, highlight bergerak juga → otak langsung mikir **"oh ini emas asli yang reflektif"**. Bikin dua-duanya bergerak = efek dramatis maksimal.

**Bedanya dengan DEFAULT:**
- Hanya 1 model (bukan 6) → fokus penuh ke satu material
- Kamera fixed (bukan FPS) → komposisi shot terjamin & sinematik
- Light orbit 360° radius lebih besar (3.0 vs 1.5) → highlight lebih dramatis
- Rotasi model di sumbu Y murni (bukan miring) → seperti turntable display di toko

---

## Glosarium Cepat (untuk yang lupa istilah)

| Istilah | Bayangkan... |
|---|---|
| **Pipeline** | Pabrik berjenjang: bahan mentah masuk ujung satu, produk jadi keluar ujung satunya |
| **Shader** | Program kecil yang jalan di GPU, bukan CPU |
| **Vertex** | Satu titik di permukaan model |
| **Fragment / Piksel** | Satu kotak warna di layar |
| **Mesh** | Kumpulan segitiga yang membentuk permukaan model |
| **Normal** | Lidi tegak lurus ke permukaan, menunjuk "arah hadap" |
| **VAO/VBO/EBO** | Folder penyimpanan data vertex di GPU (kotak data + petunjuk pemasangan) |
| **Uniform** | Variabel yang sama untuk semua vertex/fragment dalam satu draw call — kayak setting "kuas" pelukis untuk satu lukisan |
| **World space** | Koordinat di "studio dunia nyata" |
| **Local space** | Koordinat di "boneka mentah" sebelum ditaruh di studio |
| **Clip space** | Koordinat akhir yang siap diproyeksikan ke layar |
| **FOV (Field of View)** | Sudut bukaan lensa — kecil = telephoto/zoom, besar = wide-angle |
| **Perspective** | Lensa kamera normal: yang jauh kelihatan kecil |
| **Orthographic** | Lensa blueprint: yang jauh tetap sama besar |
| **Depth buffer** | Catatan "piksel ini jaraknya berapa" supaya yang di belakang tidak menimpa yang di depan |
| **Foreshortening** | Efek "menyempit di kejauhan" — kayak rel kereta menyatu di horizon |
| **Aspect ratio** | Rasio lebar:tinggi layar — 16:9, 4:3, dll |

---

## Ringkasan File

```
src/
├── main.cpp       — entry, render loop, scene mode switch, material preset
├── Shader.cpp/h   — wrapper kompilasi + uniform setter
├── Camera.cpp/h   — FPS camera (Yaw/Pitch + WASD/mouse)
├── Model.cpp/h    — load .obj/.glb via Assimp, recursive node traversal
└── Mesh.cpp/h     — VAO/VBO/EBO setup + Draw

shaders/
├── phong.vert     — pass posisi world-space + normal matrix
└── phong.frag     — Phong (ambient + diffuse + specular)
```

## Build & Run
```bash
cd build
cmake ..
make -j
./app
```

**Kontrol:**
- `W`/`A`/`S`/`D` — gerak kamera
- Mouse — look around
- Scroll — zoom (FOV)
- `1` — Default perspective + FPS (6 material)
- `2` — Orthographic (6 material)
- `3` — Top-down (6 material)
- `4` — Showcase: 1 model emas, model & light berputar
- `ESC` — keluar
