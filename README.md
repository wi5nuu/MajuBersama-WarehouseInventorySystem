# MajuBersama Warehouse Inventory Management System

Sistem manajemen inventaris gudang enterprise berbasis **C++20** dengan arsitektur **Clean Architecture + DDD**, **PostgreSQL**, dan **REST API** asinkronus.

> **Perusahaan:** PT. Maju Bersama  
> **Platform:** Linux (Ubuntu 22.04 / Debian 12) — juga kompatibel Windows (MinGW)  
> **Lisensi:** Proprietary

---

## Fitur

| Modul | Fitur |
|-------|-------|
| REST API | CRUD lengkap (GET/POST/PUT/DELETE/PATCH) untuk item inventaris |
| Autentikasi | JWT access token (15 menit) + refresh token (7 hari) + RBAC (Admin/Manager/Operator/Viewer) |
| Validasi | Custom input validator dengan error response RFC 7807 (Problem Details) |
| Database | Connection pooling, parameterized query, migration system otomatis |
| Logging | Structured logging spdlog — level DEBUG/INFO/WARN/ERROR/CRITICAL + file rotation |
| Testing | Google Test — 168 unit test case, integration test (DISABLED, butuh DB) |
| Cache | In-memory cache adapter dengan TTL |
| Messaging | In-memory message broker (pub/sub per topic) |
| Error Handling | `Result<T, Error>` — zero exception di business logic |

---

## Stack Teknologi

| Komponen | Teknologi |
|----------|-----------|
| **Bahasa** | C++20 (`-Wall -Wextra -Wpedantic -O2`) |
| **Build** | CMake 3.27+ / Ninja |
| **Compiler** | GCC 13+ atau MinGW-w64 |
| **Database** | PostgreSQL 16 + libpqxx 7.x |
| **HTTP Framework** | Drogon (async, C++17+) |
| **Format** | fmt / `std::format` |
| **Logging** | spdlog (console + rotating file) |
| **Config** | YAML (yaml-cpp) |
| **JSON** | nlohmann-json |
| **JWT** | jwt-cpp (Thalhammer) |
| **Kriptografi** | OpenSSL (SHA-256) |
| **Testing** | Google Test + Google Mock |
| **Arsitektur** | Clean Architecture + Domain-Driven Design |

---

## Struktur Proyek

```
MajuBersama-WarehouseInventorySystem/
├── CMakeLists.txt              # Root build config
├── Makefile                    # Shortcut commands
├── README.md
├── ARCHITECTURE.md             # Keputusan desain arsitektur
├── .clang-format               # Code style rules
├── .clang-tidy                 # Static analysis rules
├── .gitignore
│
├── cmake/
│   ├── FindDependencies.cmake
│   └── CompilerOptions.cmake
│
├── config/
│   ├── config.yaml.example     # Template konfigurasi
│   └── logging.yaml.example    # Template logging
│
├── src/
│   ├── main.cpp                # Entry point + DI setup
│   │
│   ├── common/                 # Shared utilities
│   │   ├── result.hpp          # Result<T, Error> type
│   │   ├── logger.hpp          # Logging wrapper (spdlog)
│   │   ├── config.hpp          # Config loader (YAML)
│   │   └── utils/
│   │       ├── uuid.hpp        # UUID v4 generator
│   │       └── time_utils.hpp  # ISO 8601 formatter/parser
│   │
│   ├── core/                   # Domain layer (murni C++, tanpa framework)
│   │   ├── entities/           # Item, User, Warehouse, Category, Supplier
│   │   ├── value_objects/      # Money, Quantity, Email, Address
│   │   ├── repositories/       # Interface repositori
│   │   ├── services/           # Interface service (Auth, Inventory)
│   │   └── use_cases/          # Business logic (CreateItem, GetItem, dll)
│   │
│   ├── infrastructure/         # Framework, DB, external adapters
│   │   ├── database/           # Connection pool, migration system
│   │   ├── repositories/       # PostgresItemRepository, PostgresUserRepository
│   │   ├── cache/              # InMemoryCacheAdapter
│   │   └── messaging/          # InMemoryMessageBroker
│   │
│   └── api/                    # HTTP layer (Drogon)
│       ├── controllers/        # ItemController, AuthController
│       ├── middleware/         # AuthMiddleware (JWT), LoggingMiddleware
│       ├── dto/                # Request/Response DTOs
│       └── validators/         # ItemValidator
│
├── tests/
│   ├── unit/                   # 14 file — 168 test case
│   ├── integration/            # Integration test (butuh DB)
│   └── fixtures/               # Test data SQL + mock helpers
│
└── docs/
    ├── api.yaml                # OpenAPI 3.0 spec
    └── diagrams/               # PlantUML (architecture, sequence, data model)
```

---

## Prasyarat

| Dependency | Minimum | Install (Ubuntu/Debian) |
|------------|---------|------------------------|
| GCC | 13+ | `apt install g++-13` |
| CMake | 3.27+ | `apt install cmake` |
| Ninja | — | `apt install ninja-build` |
| PostgreSQL | 16 | `apt install postgresql-16 libpq-dev` |
| libpqxx | 7.x | `apt install libpqxx-dev` |
| Drogon | — | `apt install libdrogon-dev` |
| spdlog | — | `apt install libspdlog-dev` |
| fmt | — | `apt install libfmt-dev` |
| yaml-cpp | — | `apt install libyaml-cpp-dev` |
| nlohmann-json | — | `apt install nlohmann-json-dev` |
| OpenSSL | — | `apt install libssl-dev` |
| jwt-cpp | — | Header-only (auto-fetch via CMake) |
| Google Test | — | `apt install libgtest-dev` |

---

## Setup Database

```bash
# Linux
sudo -u postgres psql -c "CREATE DATABASE inventory_db;"
sudo -u postgres psql -c "CREATE USER inventory_user WITH PASSWORD 'passwordmu';"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE inventory_db TO inventory_user;"
```

```powershell
# Windows (PostgreSQL 18 di port 5433)
& "C:\Program Files\PostgreSQL\18\bin\psql.exe" -U postgres -p 5433 -c "CREATE DATABASE inventory_db;"
& "C:\Program Files\PostgreSQL\18\bin\psql.exe" -U postgres -p 5433 -c "CREATE USER inventory_user WITH PASSWORD 'passwordmu';"
& "C:\Program Files\PostgreSQL\18\bin\psql.exe" -U postgres -p 5433 -c "GRANT ALL PRIVILEGES ON DATABASE inventory_db TO inventory_user;"
```

---

## Konfigurasi

Salin dan edit:

```bash
cp config/config.yaml.example config/config.yaml
nano config/config.yaml
```

Parameter penting:

```yaml
database:
  host: "localhost"
  port: 5432             # default PostgreSQL; ganti 5433 jika perlu
  name: "inventory_db"
  user: "inventory_user"
  password: "passwordmu"  # isi password

jwt:
  access_secret: "${JWT_ACCESS_SECRET}"   # set environment variable
  refresh_secret: "${JWT_REFRESH_SECRET}"
```

> **Security:** Password dan secret di `config.yaml` pakai `${ENV_VAR}` yang akan di-resolve dari environment variable saat runtime.

---

## Build & Run

### Linux

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
./inventory_management_system
```

### Windows (MinGW)

```powershell
mkdir build; cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
.\inventory_management_system.exe
```

Migration database akan otomatis dijalankan saat aplikasi start.

---

## Testing

```bash
# Semua test (168 unit test)
cd build && ctest --output-on-failure

# Test spesifik
./build/test_result
./build/test_money
```

---

## API Reference

**Base URL:** `http://localhost:8080/api/v1`

### Autentikasi

Default admin user:

| Username | Password | Role |
|----------|----------|------|
| `admin` | `admin123` | Admin |

**Login:**

```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'
```

Response:

```json
{
  "access_token": "eyJhbG...",
  "refresh_token": "eyJhbG...",
  "user": { "id": "...", "username": "admin", "role": "admin" }
}
```

**Refresh Token:**

```bash
curl -X POST http://localhost:8080/api/v1/auth/refresh \
  -H "Content-Type: application/json" \
  -d '{"refresh_token": "eyJhbG..."}'
```

### Items

Semua endpoint `/items` membutuhkan header:

```
Authorization: Bearer <access_token>
```

| Method | Path | Deskripsi | Role |
|--------|------|-----------|------|
| `GET` | `/items` | List item (dengan filter) | Viewer+ |
| `POST` | `/items` | Buat item baru | Operator+ |
| `GET` | `/items/{id}` | Detail item | Viewer+ |
| `PUT` | `/items/{id}` | Update item lengkap | Operator+ |
| `PATCH` | `/items/{id}` | Update item parsial | Operator+ |
| `DELETE` | `/items/{id}` | Hapus item | Manager+ |

**Contoh: Buat Item**

```bash
curl -X POST http://localhost:8080/api/v1/items \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "sku": "LAP001",
    "name": "Laptop 14 inch",
    "description": "Laptop kantor 14 inch Intel i7",
    "category_id": "b0000000-0000-0000-0000-000000000001",
    "supplier_id": "c0000000-0000-0000-0000-000000000001",
    "unit_price_amount": 15000000,
    "currency": "IDR",
    "stock_quantity": 50,
    "min_stock_quantity": 10
  }'
```

**Contoh: List Item dengan Filter**

```bash
curl "http://localhost:8080/api/v1/items?page=1&page_size=20&search=laptop&status=active"
```

**Error Response (RFC 7807):**

```json
{
  "type": "about:blank",
  "title": "Validation Error",
  "status": 422,
  "detail": "Name cannot be empty",
  "instance": "/api/v1/items"
}
```

---

Dokumentasi OpenAPI lengkap tersedia di [`docs/api.yaml`](docs/api.yaml).  
Diagram arsitektur tersedia di [`docs/diagrams/`](docs/diagrams/).
