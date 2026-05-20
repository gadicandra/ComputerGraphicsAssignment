# Penjelasan Lengkap Kode TVG (Shaders + Source + main.cpp)

Dokumen ini menjelaskan **apa fungsi setiap file**, **kenapa ditulis seperti itu**, dan yang paling penting: **apa yang akan terjadi ke hasil rendering kalau kita ubah/tambah komponen tertentu** (shader, lighting, material, transform, dll).

Dibaca dari atas ke bawah → bird's-eye view → detail per file → main loop → eksperimen "what-if".

---

## 1. Arsitektur Bird's-Eye View

Pipeline ini klasik tutorial-style OpenGL: **CPU side** (C++) menyiapkan geometri + matriks + uniform → upload ke GPU → **GPU side** (GLSL shader) menghitung posisi pixel + warna pixel.

```
main.cpp  ──┬─→ Shader (compile GLSL, set uniform)
            ├─→ Camera (matriks view + input handling)
            └─→ Model ──→ Mesh (VAO/VBO/EBO, draw call)
                    ↑
                 Assimp (parse .glb/.obj)

shaders/phong.vert  → posisi vertex di clip space + bawa data ke fragment
shaders/phong.frag  → hitung warna pixel pakai Phong lighting model
```

File-file di `src/` adalah wrapper tipis di atas OpenGL API supaya `main.cpp` tetap deklaratif dan mudah dibaca.

---

## 2. Shaders — di mana lighting & posisi benar-benar dihitung

### 2.1 `shaders/phong.vert` (Vertex Shader)

Dipanggil **sekali per vertex**. Tugasnya dua:

1. **Transform posisi** dari local-space ke clip-space lewat rantai `projection * view * model`. Ini pipeline koordinat standar OpenGL:
   - `model` menempatkan objek di world
   - `view` memindahkan world relatif ke kamera
   - `projection` melakukan perspective/ortho

2. **Hand-off data ke fragment shader**:
   - `FragPos` = posisi vertex di **world space** (sudah dikalikan `model`, tapi belum `view/projection`). Wajib world-space karena fragment shader nanti hitung jarak ke lampu di world space juga.
   - `Normal` = arah normal, tapi di-transform pakai `mat3(transpose(inverse(model)))`. Ini bukan basa-basi — kalau ada **non-uniform scaling** di `model` (misal `scale(2,1,1)`), normal yang cuma dikali `model` akan ter-shear dan jadi salah arah → lighting jadi miring. Trik `inverse-transpose` membatalkan efek scaling sehingga normal tetap tegak lurus permukaan.

### 2.2 `shaders/phong.frag` (Fragment Shader)

Dipanggil **sekali per pixel**. Implementasi tekstual dari **Phong reflection model** — superposisi 3 komponen:

| Komponen | Intuisi | Rumus inti |
|---|---|---|
| **Ambient** | "Cahaya nyasar" dari pantulan global. Bikin sisi yang tidak kena lampu tidak pitch-black. | `light.ambient * material.ambient` |
| **Diffuse** | Lambertian — semakin tegak lurus normal ke arah lampu, semakin terang. Ini yang bikin objek terlihat punya bentuk 3D. | `max(dot(N, L), 0) * material.diffuse` |
| **Specular** | Kilau / highlight — yang bikin emas terlihat metalik, plastik glossy. Bergantung sudut pandang. | `pow(max(dot(V, R), 0), shininess) * material.specular` |

`shininess` adalah eksponen → makin tinggi → highlight makin kecil & tajam (metalik). Makin rendah → highlight melar (rubber/matte).

**Kenapa Phong, bukan PBR?** Phong adalah model empiris pre-2010-an, tidak energy-conserving, tapi cukup untuk demo material klasik dan jauh lebih sederhana dari Cook-Torrance/GGX. Untuk tujuan akademis, ini perfect.

---

## 3. Source Files (`src/`)

### 3.1 `Shader.{h,cpp}` — RAII wrapper untuk GLSL program
- **Constructor**: baca file `.vert`/`.frag` → `glCompileShader` → `glLinkProgram` → simpan ID. `checkCompile` cetak error compile/link supaya tidak silent-fail (favorite footgun OpenGL).
- **`setVec3/setMat4/...`**: tipis di atas `glUniform*`. Dipanggil tiap frame dari `main.cpp` untuk push data terbaru (lampu, kamera, material) ke GPU. Lookup uniform pakai `glGetUniformLocation` tiap call — tidak di-cache, jadi sedikit boros call, tapi tidak masalah untuk demo.

### 3.2 `Camera.{h,cpp}` — FPS-style flying camera
Implementasi tutorial LearnOpenGL klasik:
- Simpan `Position`, `Front`, `Up`, `Right` (basis ortonormal lokal kamera) + `Yaw`/`Pitch` (sudut euler).
- `updateVectors()` rekonstruksi `Front` dari Yaw/Pitch pakai spherical-to-cartesian; `Right` dan `Up` lewat cross product. Trik klasik: kunci pitch ke ±89° supaya tidak gimbal-lock saat lihat tegak ke atas/bawah.
- `GetViewMatrix()` cuma wrapper `glm::lookAt(pos, pos+front, up)`.
- `ProcessKeyboard` gerakkan posisi sepanjang `Front`/`Right`, dikalikan `deltaTime` → frame-rate independent.
- `ProcessMouseScroll` bukan zoom optik — dia ubah `Zoom` (FOV degrees), dipakai di `glm::perspective`. Jadi scroll = ubah lensa, bukan dolly.

### 3.3 `Mesh.{h,cpp}` — satu unit drawable
- `struct Vertex` = `{Position, Normal, TexCoords}` interleaved → satu VBO, satu stride. Cache-friendly.
- `setupMesh()`: bikin **VAO** (state container untuk attribute layout), **VBO** (data vertex), **EBO** (indices). Indexing wajib untuk model real karena banyak vertex shared antar triangle.
- 3 attribute location: pos (0), normal (1), texcoord (2). Texcoord disiapkan tapi belum dipakai shader — antisipasi extension nanti.
- `Draw()` cuma `glBindVertexArray + glDrawElements`. Param `Shader&` tidak dipakai sekarang — sengaja, supaya kalau nanti mesh punya texture sendiri, dia bisa set sampler uniform tanpa ganti signature.

### 3.4 `Model.{h,cpp}` — multi-mesh loader pakai Assimp
Assimp di-config dengan 4 flag penting:
- `Triangulate` — semua face dipecah ke triangle (OpenGL native cuma triangle).
- `GenSmoothNormals` — fallback kalau .obj/.glb tidak punya normal. Tanpa ini, shader hitung lighting pakai garbage.
- `FlipUVs` — Assimp/most tools pakai UV origin di kiri-atas, OpenGL kiri-bawah. Tanpa flip, tekstur muncul kebalik vertikal.
- `JoinIdenticalVertices` — dedup vertex untuk indexing efisien.

`processNode` rekursif menelusuri scene graph dan flatten semua mesh ke `std::vector<Mesh>`. **Penting**: model hierarchy/transforms node-level **diabaikan** — semua mesh dianggap di local-origin yang sama. Untuk model simple OK; untuk model rigged/multi-part bisa salah posisi.

`processMesh` copy data Assimp ke `Vertex` struct kita. Material Assimp **tidak dibaca** — material di-override total dari `MATERIALS[]` di `main.cpp`.

### 3.5 `stb_image_impl.cpp` — single-translation-unit trick
`stb_image.h` adalah header-only library. Macro `STB_IMAGE_IMPLEMENTATION` di-define **persis di satu file** supaya implementasi jadi simbol nyata (bukan inline di tiap include). Kalau lupa, dapat linker error "undefined reference". Belum dipakai aktif karena shader belum sample texture, tapi siap untuk extension.

---

## 4. `main.cpp` — Walkthrough lengkap

### 4.1 Globals & callback (baris 11–42)
```cpp
Camera camera(glm::vec3(0.0f, 2.0f, 15.0f));
```
Kamera di-instantiate sebagai **global** karena GLFW callback adalah C-style function pointer — tidak punya `this`. Global = hack standar untuk passing state ke callback. Posisi awal `(0, 2, 15)` artinya kamera mundur 15 unit di +Z dan naik 2 unit → cukup jauh untuk lihat semua 6 model sekaligus.

`firstMouse` flag mencegah kamera "loncat" di frame pertama: tanpa ini, `lastX/lastY` masih di tengah layar, tapi cursor real sudah entah di mana — selisihnya jadi gerak besar mendadak.

`processInput`:
- WASD → gerakkan kamera.
- Tombol 1–4 → switch scene mode. State machine sederhana untuk demo 4 konfigurasi visualisasi.

### 4.2 Material presets (baris 44–62)
Tabel 6 material klasik dari devernay.free.fr — sama persis dengan tutorial OpenGL lawas.

**Dampak visual tiap preset**:
- `gold` (0.4 shininess): highlight medium-sharp, warna kuning hangat.
- `silver`: grayscale, mirip gold tapi tidak berwarna — bentuk kelihatan abu-abu metalik.
- `ruby/emerald`: diffuse merah/hijau kuat + specular hampir putih → permata terlihat translucent-like.
- `cyan_plastic`: shininess 0.25 → highlight melar = plastik glossy.
- `black_rubber`: hampir gelap total, shininess 0.078 → matte pekat.

`shininess * 128.0f` adalah konvensi historis: tabel disimpan 0–1, dikali 128 untuk eksponen Phong realistic.

### 4.3 GLFW + GLAD init (baris 65–78)
Boilerplate standar:
- Context OpenGL 4.6 Core Profile (no legacy fixed-function).
- `GLFW_CURSOR_DISABLED` → cursor hidden + locked ke window center → infinite mouse delta = FPS camera.
- `glEnable(GL_DEPTH_TEST)` — **wajib untuk 3D**. Tanpa ini, mesh terakhir di-draw akan menimpa yang di depannya (urutan submission, bukan depth) → model acak-acakan.

### 4.4 Asset load (baris 80–83)
- Shader dikompilasi dari 2 file GLSL.
- 1 model GLB di-load. Karena hanya 1 instance Model dipakai berulang dengan transform berbeda → manual "instancing" (tetap N draw call, bukan true instancing).

### 4.5 Cube vertices (baris 86–142)
Ada 36 vertex untuk kubus + setup VAO. **Ini dead code** — `cubeVAO` di-setup tapi tidak pernah di-bind/di-draw di main loop. Sisa eksperimen awal. Tidak mempengaruhi visual.

### 4.6 Main loop (baris 145–266)

**Step 1 — Timing**:
```cpp
deltaTime = now - lastFrame;
```
Dipakai di `processInput` → kamera bergerak konstan terlepas dari FPS.

**Step 2 — Clear**: background gelap + clear depth buffer. Depth clear wajib tiap frame.

**Step 3 — Pilih projection + view matrix berdasarkan sceneMode**:

| Mode | Projection | View | Dampak modeling |
|---|---|---|---|
| **DEFAULT** | `perspective(camera.Zoom, aspect, 0.1, 100)` | `camera.GetViewMatrix()` (free WASD) | Foreshortening normal — model jauh = kecil. User bisa terbang. |
| **ORTHO** | `ortho(-8a, 8a, -8, 8)` | fixed `lookAt((0,0,10) → origin)` | **Tidak ada foreshortening** → model jauh = sama besar. Cocok untuk blueprint. Material terlihat lebih "flat". |
| **TOPDOWN** | `perspective(60°, ...)` | dari atas `(0,8,0.001)` lihat ke origin | Kamera dari atas. `0.001` di Z biar `lookAt` tidak degenerate. Model di-rotate 90° di X supaya wajah depan menghadap kamera atas. |
| **SHOWCASE** | `perspective(45°, ...)` | fixed cinematic `(0,1,4) → origin` | Kamera diam, model gold di tengah, model + lampu berputar — highlight menyapu = pamer efek metalik. |

**Step 4 — Upload uniforms**: push 3 matrix ke shader.

**Step 5 — Animate light**:
- **SHOWCASE**: lampu orbit penuh 360° radius 3, naik turun di Y. Tujuan: highlight emas **menyapu** permukaan saat lampu lewat → mata menafsirkan sebagai "shiny". Tanpa gerakan lampu, highlight emas cuma spot statis = lebih mirip cat kuning.
- **Mode lain**: lampu osilasi pelan di X dan Z, Y fixed.

**Step 6 — Light intensity**:
```cpp
ambient  = 0.4
diffuse  = 0.8
specular = 1.0
```
Tabel material klasik **mengasumsikan lampu full-white**. Kalau diffuse terlalu rendah, warna `gold.diffuse = (0.75, 0.60, 0.22)` dipotong → emas gelap, bukan keemasan.

**Step 7 — Render branching**:

**SHOWCASE**: 1 model di center, rotate sumbu Y, `scale(0.25)` karena kamera dekat (z=4). Material di-set ke gold.

**Default/Ortho/Topdown**: loop 6 material, translate horizontal `x = (i - 3) * 2.5`. Tiap iterasi swap material → 6 material berdampingan untuk perbandingan visual langsung. `scale(0.02)` karena di mode ini kamera jauh dan harus muat 6 model.

**Step 8 — Swap & poll**: standard double buffer.

---

## 5. "What-If" — Eksperimen & Dampaknya ke Modeling

Bagian ini adalah inti dari dokumen ini: **kalau kita ubah komponen X, apa yang akan terlihat di model?** Berguna untuk eksperimen tanpa harus run-and-see berulang kali.

### 5.1 Eksperimen di Fragment Shader (`phong.frag`)

#### A. Matikan komponen Ambient → `vec3 ambient = vec3(0.0);`
- **Dampak**: sisi model yang tidak menghadap lampu jadi **pitch-black total**. Detail di area bayangan hilang.
- **Gunanya kapan?** Untuk render dramatik (chiaroscuro), atau saat mau gabung dengan ambient occlusion map.

#### B. Matikan Diffuse → `vec3 diffuse = vec3(0.0);`
- **Dampak**: model kehilangan **sense of 3D shape**. Yang tersisa cuma highlight specular + ambient seragam. Hasilnya terlihat seperti foil/cermin abstrak, tidak ada gradient lembut antara terang-gelap.
- **Insight**: diffuse adalah komponen yang paling kuat mengomunikasikan "bentuk" objek ke mata.

#### C. Matikan Specular → `vec3 specular = vec3(0.0);`
- **Dampak**: semua material kelihatan **matte/plaster**. Emas akan kelihatan seperti karton kuning, ruby seperti plastisin merah. Tidak ada perbedaan jelas antara metal dan rubber lagi.
- **Insight**: specular yang membedakan material kelas "metallic/glossy" vs "matte".

#### D. Naikkan shininess di kode → `mat.shininess * 256.0f` (bukan 128)
- **Dampak**: highlight jadi **lebih tajam dan kecil** — kesan metalik high-polish (mirror-finish). Cocok untuk simulasi chrome.
- **Kebalikan**: turunkan ke `* 32.0f` → highlight melar besar → mirip plastik murah / kulit jeruk.

#### E. Ganti rumus diffuse jadi `pow(diff, 0.5)` (gamma-like)
- **Dampak**: gradient terang-gelap jadi lebih halus, bayangan lebih terang. Mirip efek Lambert-warp di stylized rendering / anime shader.

#### F. Tambah komponen Rim Light (cell-shading-ish):
```glsl
float rim = 1.0 - max(dot(viewDir, norm), 0.0);
vec3 rimColor = vec3(0.3, 0.5, 1.0) * pow(rim, 3.0);
vec3 result = ambient + diffuse + specular + rimColor;
```
- **Dampak**: model akan punya **garis cahaya tipis di siluetnya** (di pinggiran yang menghadap miring dari kamera). Bikin model "lepas" dari background — efek populer untuk character art atau sci-fi.

#### G. Toon/Cell shading — step the diffuse:
```glsl
diff = floor(diff * 4.0) / 4.0;
```
- **Dampak**: gradient lembut diganti **band-band warna terpotong** (4 level di sini). Hasilnya gaya cartoon/anime. Specular bisa juga di-step dengan `step(0.5, spec)` untuk highlight binary.

#### H. Tambah attenuation jarak lampu:
```glsl
float dist = length(light.position - FragPos);
float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
diffuse  *= attenuation;
specular *= attenuation;
```
- **Dampak**: lampu jadi **point light realistis** — model yang jauh dari lampu gelap, yang dekat terang. Tanpa ini, lampu kita sebenarnya "infinite intensity" yang menyala sama kuat di mana saja.

#### I. Tambah second light (directional, simulasi matahari):
```glsl
vec3 sunDir = normalize(vec3(-0.3, -1.0, -0.5));
float sunDiff = max(dot(norm, -sunDir), 0.0);
vec3 sunColor = vec3(1.0, 0.95, 0.8) * sunDiff * 0.4;
result += sunColor * material.diffuse;
```
- **Dampak**: scene punya **fill light hangat** dari satu arah konstan + key light yang bergerak (lampu lama). Bayangan tidak lagi pitch-black di area yang menghadap lampu utama. Lebih sinematik.

### 5.2 Eksperimen di Vertex Shader (`phong.vert`)

#### A. Hapus `inverse-transpose` → `Normal = mat3(model) * aNormal;`
- **Dampak terlihat saat `scale` non-uniform**: lighting **miring** — sisi yang harusnya terang jadi gelap karena normal ter-shear. Coba di `MODE_TOPDOWN` (rotate + scale) → akan terlihat.

#### B. Vertex displacement / wave deformation:
```glsl
vec3 displacedPos = aPos;
displacedPos.y += sin(aPos.x * 5.0 + time) * 0.1;
FragPos = vec3(model * vec4(displacedPos, 1.0));
```
(perlu uniform `time` ditambahkan)
- **Dampak**: model **bergelombang seperti bendera/air**. Cocok untuk efek environment. Tapi normal tidak ikut update → lighting jadi salah. Untuk benar, normal harus dihitung ulang dari derivative.

#### C. Per-vertex lighting (Gouraud, bukan Phong):
Pindahkan seluruh perhitungan lighting dari frag ke vert, lalu kirim `vec3 outColor` ke frag.
- **Dampak**: lighting **lebih murah komputasinya** tapi highlight specular jadi blocky/pixelated di model low-poly (karena di-interpolasi linear antar vertex). Bedanya jelas terlihat di scene SHOWCASE — highlight gold akan kelihatan "berkotak-kotak" alih-alih smooth.

### 5.3 Eksperimen di `main.cpp`

#### A. Naikkan `MovementSpeed` di Camera → `40.0f`
- **Dampak**: terbang seperti pesawat. Bagus untuk eksplor scene besar, tapi mudah keluar dari scene karena objek kecil.

#### B. Ganti `GL_DEPTH_TEST` jadi `glDisable(GL_DEPTH_TEST)`
- **Dampak**: model berperilaku **seperti decal 2D** — render terakhir menimpa yang sebelumnya. Di mode 6-material, model di kanan akan menimpa model di kiri tergantung urutan loop. Demonstrasi penting kenapa depth test wajib.

#### C. Tambah backface culling: `glEnable(GL_CULL_FACE);`
- **Dampak**: triangle yang menghadap menjauhi kamera tidak di-draw. **Performance naik ~50%** karena fragment shader tidak run untuk pixel yang ketutupan. **Tapi** kalau model punya orientasi triangle (winding) tidak konsisten, beberapa permukaan akan **hilang** (bolong). Test dulu sebelum enable.

#### D. Ubah `light.diffuse` jadi warna `(1.0, 0.3, 0.3)` (lampu merah)
- **Dampak**: warna material di-modulasi sama warna lampu — emas akan kelihatan **kemerahan oranye**, silver jadi **pink**. Cocok untuk simulasi sunset/horror scene.

#### E. Tambah multiple lampu yang orbit di sumbu beda:
Tambah `lightPos2`, kirim sebagai uniform `light2.position`, dan modifikasi shader untuk loop 2 light.
- **Dampak**: model punya **highlight ganda** dari arah berbeda. Sangat efektif untuk demo material — silver dengan 2 lampu warna beda kelihatan jauh lebih realistic.

#### F. Ganti material gold ke `chrome` (preset baru):
```cpp
{"chrome", {0.25f, 0.25f, 0.25f}, {0.4f, 0.4f, 0.4f}, {0.774597f, 0.774597f, 0.774597f}, 0.6f}
```
- **Dampak**: model akan terlihat **lebih reflective dari silver** — specular hampir putih murni dengan diffuse abu-abu. Tapi karena tidak ada environment reflection map, masih kelihatan "matte chrome", bukan true mirror.

#### G. Hapus `aiProcess_GenSmoothNormals` dari Model loader
- **Dampak**: kalau model .obj tidak punya normal → lighting **hitam total atau acak**. Kalau punya normal flat (per-face) → setiap face akan terlihat distinct, mirip low-poly stylized look.

#### H. Tambah animasi scale ke model:
```cpp
m = glm::scale(m, glm::vec3(0.02f + sin(now) * 0.005f));
```
- **Dampak**: model **bernapas** (besar-kecil periodic). Karena pakai `inverse-transpose` di shader, lighting tetap benar walau scale-nya berubah-ubah.

#### I. Tambahkan `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)` sebelum draw
- **Dampak**: render **wireframe** — hanya tepi triangle yang terlihat. Bagus untuk debug topology mesh atau LOD inspection.

### 5.4 Eksperimen Struktural (Tambah Sistem)

#### A. Tambah Texture sampling (skeleton sudah siap di Mesh.cpp via location 2)
1. Load tekstur via `stb_image` (sudah include).
2. Tambah `uniform sampler2D diffuseMap;` di frag shader.
3. Ganti `material.diffuse` jadi `texture(diffuseMap, TexCoords).rgb`.
- **Dampak**: model bisa pakai **tekstur asli dari GLB** (warna, detail surface). Lebih realistic, tapi material preset (gold, silver, dll) jadi tidak relevan kecuali sebagai modulator.

#### B. Tambah Shadow Mapping
1. Render scene dari sudut pandang lampu ke depth texture.
2. Di main pass, sample depth texture untuk cek apakah fragment ter-occlude.
- **Dampak**: model **menjatuhkan bayangan** ke model lain atau ke ground plane. Realisme jump besar, tapi kompleksitas naik 3-4x lipat.

#### C. Tambah Skybox
- **Dampak**: background tidak lagi flat color. Refleksi environment di material (kalau ditambah env mapping) bikin chrome/silver kelihatan true-mirror.

#### D. Migrasi dari Phong ke Blinn-Phong (1 baris):
```glsl
vec3 halfwayDir = normalize(lightDir + viewDir);
float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
```
- **Dampak**: highlight **lebih halus dan natural** terutama saat sudut tajam (grazing angle). Phong klasik bisa cut-off mendadak; Blinn-Phong tidak. Eksponen biasanya perlu dinaikkan 2-4x untuk hasil setara karena half-vector menghasilkan dot product lebih tinggi.

#### E. Migrasi penuh ke PBR (Cook-Torrance):
- **Dampak besar**: ganti `ambient/diffuse/specular/shininess` jadi `albedo/metallic/roughness/AO`. Hasil **jauh lebih realistic** dan konsisten di berbagai lighting condition. Tapi tabel `MATERIALS[]` lama harus dibuang — gold di Phong ≠ gold di PBR.

---

## 6. Ringkasan Cause-and-Effect

| Yang diubah | Dampak visual cepat |
|---|---|
| `sceneMode` (tombol 1/2/3/4) | Switch projection + view + jumlah model |
| `lightPos` animation | Highlight menyapu → kesan metalik/glossy |
| `light.ambient/diffuse/specular` | Tinggi = material jelas; rendah = semua gelap |
| `MATERIALS[i]` swap | Material per model beda (perbandingan langsung) |
| `glm::scale(...)` | Kompensasi ukuran source model |
| `inverse-transpose(model)` di vert shader | Normal tetap benar walau scale non-uniform |
| `GL_DEPTH_TEST` | Tanpa: render acak; Dengan: occlusion benar |
| Hapus komponen Phong (amb/dif/spec) | Hilang dimensi visual yang diwakili |
| Naikkan shininess | Metalik high-polish |
| Tambah attenuation | Lampu jadi realistic point light |
| Backface culling | Performance naik, risiko mesh bolong |
| Wireframe mode | Topology mesh terlihat |
| Tambah texture | Material preset jadi modulator, bukan warna utama |

Kode utama didesain untuk demo **6 material × 4 mode kamera = 24 kombinasi visual** dari satu model GLB, dengan satu pipeline Phong klasik. Setiap eksperimen di section 5 adalah variasi yang bisa langsung di-coba untuk pemahaman intuitif tentang bagaimana setiap komponen graphics pipeline mempengaruhi hasil akhir.
