# MajuBersama Warehouse Inventory Management System

**MajuBersama Warehouse Inventory Management System** adalah aplikasi enterprise backend untuk mengelola inventaris gudang secara *real-time* berbasis C++20. Dibangun dengan arsitektur **Clean Architecture + Domain-Driven Design** untuk memastikan skalabilitas, maintainability, dan ketahanan sistem pada skala industri.

Aplikasi ini menangani seluruh siklus hidup inventaris — mulai dari pencatatan barang masuk/keluar, monitoring stok minimum, manajemen supplier dan kategori, hingga autentikasi multi-level pengguna — melalui REST API yang cepat dan asinkronus.

**Perusahaan:** PT. Maju Bersama  
**Bidang:** Inventaris & Gudang  
**Platform:** Linux (Ubuntu 22.04 / Debian 12) / Windows

---

## Mengapa MajuBersama Inventory System?

| Masalah | Solusi |
|---------|--------|
| Stok gudang tidak akurat | Tracking stok *real-time* dengan minimum stock alert |
| Data tersebar di banyak file/Excel | Database terpusat PostgreSQL dengan migration system |
| Proses manual lambat & rentan error | REST API otomatis — eksekusi dalam milidetik |
| Tidak ada kontrol akses | RBAC multi-level (Admin, Manager, Operator, Viewer) |
| Sulit diintegrasikan | REST API standar + OpenAPI documentation |

---

## Arsitektur

Aplikasi memisahkan business logic murni dari infrastruktur teknis (database, HTTP, caching) menggunakan **Clean Architecture**:

```
[ Client/Apps ]
      ↓  HTTP/JSON
[ API Layer ]    ← Controllers, DTOs, Validators, Middleware
      ↓
[ Use Cases ]    ← Business logic murni (tidak tahu database)
      ↓
[ Domain Core ]  ← Entities, Value Objects, Repository Interfaces
      ↓
[ Infrastructure ] ← PostgreSQL, Cache, Message Broker
```

Keuntungan:
- Business logic bisa di-test tanpa database
- Mudah ganti database atau framework tanpa mengubah kode inti
- Kode terstruktur rapi, mudah dikembangkan tim

---

## Fitur Lengkap

### 🔐 Autentikasi & Otorisasi
- **JWT Access Token** (15 menit) + **Refresh Token** (7 hari)
- **RBAC** — 4 level peran: Admin, Manager, Operator, Viewer
- Password di-hash dengan SHA-256
- Default user: `admin` / `admin123`

### 📦 Manajemen Inventaris
- CRUD item (Create, Read, Update, Delete, Patch)
- Tracking stok *real-time* (langsung update di database)
- Minimum stock alert — notifikasi barang hampir habis
- Pencarian & filtering multi-kriteria
- Pagination untuk data besar

### 🗄️ Database & Migrasi
- PostgreSQL dengan **connection pooling** (multithread-safe)
- **Parameterized query** — 100% aman dari SQL injection
- **Migration system** otomatis — jalan saat aplikasi start
- Retry connection dengan exponential backoff

### 🌐 REST API
- Asinkronus (Drogon framework) — ribuan request simultan
- Response time < 100ms p99
- Error response standar **RFC 7807** (Problem Details)
- Validasi input otomatis
- OpenAPI 3.0 spec lengkap

### 📝 Logging
- **Structured logging** dengan level: DEBUG, INFO, WARN, ERROR, CRITICAL
- Output ke console + file rotating (max 100MB, 7 file)
- Setiap request HTTP tercatat otomatis

### 🧪 Testing
- **168 unit test** dengan Google Test
- Mock repository untuk test business logic murni
- Integration test untuk database (dijalankan terpisah)
- Coverage > 80%

---

## Prasyarat Sistem

| Dependency | Minimal | Fungsi |
|------------|---------|--------|
| GCC / MinGW-w64 | 13+ | Compiler C++20 |
| CMake | 3.27+ | Build system |
| Ninja / Make | — | Build runner |
| PostgreSQL | 16 | Database utama |
| libpqxx | 7.x | C++ client PostgreSQL |
| Drogon | latest | HTTP framework asinkronus |
| spdlog | latest | Logging |
| fmt | latest | String formatting |
| yaml-cpp | latest | Parsing konfigurasi |
| nlohmann-json | latest | Serialisasi JSON |
| OpenSSL | latest | Kriptografi (SHA-256, JWT) |
| jwt-cpp | latest | JSON Web Token (header-only) |
| Google Test | latest | Unit testing |

---

## Panduan Lengkap

### 1. Setup Database

**Linux (Ubuntu/Debian):**
```bash
sudo -u postgres psql -c "CREATE DATABASE inventory_db;"
sudo -u postgres psql -c "CREATE USER inventory_user WITH PASSWORD 'passwordmu';"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE inventory_db TO inventory_user;"
```

**Windows:**
```powershell
# Cari port PostgreSQL kmu
Get-NetTCPConnection -State Listen | Where-Object { $_.LocalPort -in (5432,5433,5434) }

# Sesuaikan port dan path dengan instalasi kmu
& "C:\Program Files\PostgreSQL\18\bin\psql.exe" -U postgres -p 5433 -c "CREATE DATABASE inventory_db;"
& "C:\Program Files\PostgreSQL\18\bin\psql.exe" -U postgres -p 5433 -c "CREATE USER inventory_user WITH PASSWORD 'passwordmu';"
& "C:\Program Files\PostgreSQL\18\bin\psql.exe" -U postgres -p 5433 -c "GRANT ALL PRIVILEGES ON DATABASE inventory_db TO inventory_user;"
```

### 2. Konfigurasi Aplikasi

```bash
cp config/config.yaml.example config/config.yaml
```

Edit `config/config.yaml` — semua parameter penting:

```yaml
server:
  host: "0.0.0.0"
  port: 8080
  workers: 4

database:
  host: "localhost"
  port: 5432            # port PostgreSQL kmu (cek jika 5433)
  name: "inventory_db"
  user: "inventory_user"
  password: "passwordmu" # ganti dengan password asli
  pool_size: 10         # koneksi simultan ke DB

jwt:
  access_secret: "rahasia-access-kmu"    # langsung atau pakai ${ENV_VAR}
  refresh_secret: "rahasia-refresh-kmu"
  access_expiry_minutes: 15
  refresh_expiry_days: 7

logging:
  level: "info"          # debug / info / warn / error
  file: "logs/app.log"
```

### 3. Build

```bash
mkdir build && cd build

# Linux
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

# Windows
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

cmake --build . --parallel
```

### 4. Jalankan

```bash
./inventory_management_system
```

Aplikasi akan:
1. Membaca `config/config.yaml`
2. Inisialisasi logging
3. Membuat koneksi ke PostgreSQL
4. **Menjalankan migration database** (membuat tabel otomatis)
5. Memasukkan user default (admin/admin123)
6. Start HTTP server di `http://0.0.0.0:8080`

### 5. Testing

```bash
cd build && ctest --output-on-failure
```

Output: 168 test cases passed.

---

## Dokumentasi API

**Base URL:** `http://localhost:8080/api/v1`

### Autentikasi

Semua request ke `/items` membutuhkan token JWT.

**Login — dapatkan token:**

```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'
```

**Response:**
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "refresh_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user": {
    "id": "00000000-0000-0000-0000-000000000001",
    "username": "admin",
    "email": "admin@maju-bersama.co.id",
    "role": "admin"
  }
}
```

**Refresh token** (saat access token expired):

```bash
curl -X POST http://localhost:8080/api/v1/auth/refresh \
  -H "Content-Type: application/json" \
  -d '{"refresh_token": "eyJhbGciOiJIUzI1NiJ9..."}'
```

### Items — Endpoint Lengkap

Setiap request menyertakan header:
```
Authorization: Bearer eyJhbGciOiJIUzI1NiJ9...
```

#### GET /items — List Semua Item

```bash
curl "http://localhost:8080/api/v1/items?page=1&page_size=20&search=laptop&status=active&category_id=..."
```

Parameter filter:
| Parameter | Tipe | Contoh |
|-----------|------|--------|
| `page` | integer | `1` |
| `page_size` | integer | `20` |
| `search` | string | `laptop` |
| `category_id` | uuid | `b000...` |
| `status` | enum | `active`, `inactive`, `discontinued`, `out_of_stock` |
| `min_stock` | integer | `10` |
| `max_price` | integer | `5000000` |

#### POST /items — Buat Item Baru

```bash
curl -X POST http://localhost:8080/api/v1/items \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "sku": "LAP001",
    "name": "Laptop 14 inch",
    "description": "Laptop kantor 14 inch Intel i7, RAM 16GB, SSD 512GB",
    "category_id": "b0000000-0000-0000-0000-000000000001",
    "supplier_id": "c0000000-0000-0000-0000-000000000001",
    "unit_price_amount": 15000000,
    "currency": "IDR",
    "stock_quantity": 50,
    "min_stock_quantity": 10,
    "stock_unit": "pcs"
  }'
```

**Response (201 Created):**
```json
{
  "id": "e0000000-0000-0000-0000-000000000001",
  "sku": "LAP001",
  "name": "Laptop 14 inch",
  "description": "Laptop kantor 14 inch Intel i7, RAM 16GB, SSD 512GB",
  "category_id": "b0000000-0000-0000-0000-000000000001",
  "supplier_id": "c0000000-0000-0000-0000-000000000001",
  "unit_price": { "amount": 15000000, "currency": "IDR" },
  "stock": { "quantity": 50, "unit": "pcs" },
  "min_stock": { "quantity": 10, "unit": "pcs" },
  "status": "active",
  "created_at": "2026-06-01T10:30:00.000Z",
  "updated_at": "2026-06-01T10:30:00.000Z"
}
```

#### GET /items/{id} — Detail Item

```bash
curl http://localhost:8080/api/v1/items/e0000000-0000-0000-0000-000000000001 \
  -H "Authorization: Bearer <token>"
```

#### PUT /items/{id} — Update Item (semua field)

```bash
curl -X PUT http://localhost:8080/api/v1/items/e0000000-0000-0000-0000-000000000001 \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "sku": "LAP001",
    "name": "Laptop 14 inch Updated",
    "unit_price_amount": 14000000,
    "stock_quantity": 45
  }'
```

#### PATCH /items/{id} — Update Sebagian Field

```bash
curl -X PATCH http://localhost:8080/api/v1/items/e0000000-0000-0000-0000-000000000001 \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{"unit_price_amount": 13500000, "stock_quantity": 60}'
```

#### DELETE /items/{id} — Hapus Item

```bash
curl -X DELETE http://localhost:8080/api/v1/items/e0000000-0000-0000-0000-000000000001 \
  -H "Authorization: Bearer <token>"
```

**Response:** 204 No Content

### Error Response Format (RFC 7807)

Semua error mengikuti standar RFC 7807 Problem Details:

| Status | Contoh |
|--------|--------|
| 400 | `{"title":"Bad Request","detail":"Invalid JSON"}` |
| 401 | `{"title":"Unauthorized","detail":"Invalid token"}` |
| 404 | `{"title":"Not Found","detail":"Item not found"}` |
| 409 | `{"title":"Conflict","detail":"SKU already exists"}` |
| 422 | `{"title":"Validation Error","detail":"Name cannot be empty"}` |
| 500 | `{"title":"Internal Server Error","detail":"..."}` |

---

## RBAC — Hak Akses Per Peran

| Role | View Item | Create Item | Edit Item | Delete Item | Manage Users |
|------|-----------|-------------|-----------|-------------|--------------|
| **Admin** | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Manager** | ✅ | ✅ | ✅ | ✅ | ❌ |
| **Operator** | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Viewer** | ✅ | ❌ | ❌ | ❌ | ❌ |

---

## Performa & Keandalan

| Aspek | Detail |
|-------|--------|
| **Concurrency** | Thread pool dengan `std::jthread` (C++20), cancelable via stop_token |
| **Koneksi DB** | Connection pooling (multithread-safe), max retry 3x dengan exponential backoff |
| **SQL** | 100% parameterized query — zero SQL injection |
| **Memory** | Zero raw pointer — `std::unique_ptr`, `std::shared_ptr` |
| **Error** | `Result<T, Error>` pattern — zero exception di business logic |
| **Startup** | Migration database otomatis — siap pakai dalam detik |

---

Dokumentasi API OpenAPI 3.0: [`docs/api.yaml`](docs/api.yaml)  
Diagram arsitektur & data model: [`docs/diagrams/`](docs/diagrams/)
