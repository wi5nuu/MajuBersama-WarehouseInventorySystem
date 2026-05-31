# MajuBersama Warehouse Inventory Management System

Sistem manajemen inventaris gudang enterprise berbasis **C++20** dengan **Clean Architecture**, **PostgreSQL**, dan **REST API** asinkronus.

**Perusahaan:** PT. Maju Bersama

---

## Fitur

- REST API CRUD item inventaris (GET/POST/PUT/DELETE/PATCH)
- Autentikasi JWT (access + refresh token) + RBAC (Admin/Manager/Operator/Viewer)
- Validasi input custom dengan error response RFC 7807
- Database PostgreSQL dengan connection pooling & migration otomatis
- Structured logging spdlog (file rotation)
- 168 unit test (Google Test)
- Caching in-memory + message broker in-memory
- Error handling `Result<T, Error>` — zero exception

---

## Prasyarat

| Dependency | Minimal |
|------------|---------|
| GCC / MinGW-w64 | 13+ |
| CMake | 3.27+ |
| Ninja / Make | — |
| PostgreSQL | 16 |
| libpqxx | 7.x |
| Drogon | latest |
| spdlog, fmt, yaml-cpp | latest |
| nlohmann-json, OpenSSL | latest |
| jwt-cpp | header-only (auto-fetch) |
| Google Test | latest |

---

## Setup Database

```bash
# Linux
sudo -u postgres psql -c "CREATE DATABASE inventory_db;"
sudo -u postgres psql -c "CREATE USER inventory_user WITH PASSWORD 'passwordmu';"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE inventory_db TO inventory_user;"
```

```powershell
# Windows (cek port PostgreSQL kmu)
& "C:\Program Files\PostgreSQL\18\bin\psql.exe" -U postgres -p 5433 -c "CREATE DATABASE inventory_db;"
& "C:\Program Files\PostgreSQL\18\bin\psql.exe" -U postgres -p 5433 -c "CREATE USER inventory_user WITH PASSWORD 'passwordmu';"
& "C:\Program Files\PostgreSQL\18\bin\psql.exe" -U postgres -p 5433 -c "GRANT ALL PRIVILEGES ON DATABASE inventory_db TO inventory_user;"
```

---

## Konfigurasi

```bash
cp config/config.yaml.example config/config.yaml
```

Edit `config/config.yaml`:

```yaml
database:
  host: "localhost"
  port: 5432            # ganti sesuai port PostgreSQL kmu
  name: "inventory_db"
  user: "inventory_user"
  password: "passwordmu"

jwt:
  access_secret: "${JWT_ACCESS_SECRET}"    # isi langsung atau pakai env var
  refresh_secret: "${JWT_REFRESH_SECRET}"
```

---

## Build & Run

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
./inventory_management_system
```

Migration database otomatis jalan saat aplikasi start.

---

## Testing

```bash
cd build && ctest --output-on-failure
```

---

## API Reference

**Base URL:** `http://localhost:8080/api/v1`

### Login (default: admin / admin123)

```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'
```

Response:

```json
{
  "access_token": "eyJ...",
  "refresh_token": "eyJ...",
  "user": { "id": "...", "username": "admin", "role": "admin" }
}
```

### Items (wajib header `Authorization: Bearer <token>`)

| Method | Endpoint | Deskripsi | Minimal Role |
|--------|----------|-----------|-------------|
| GET | `/items` | List item (filter: `?search=&status=&page=&page_size=`) | Viewer |
| POST | `/items` | Buat item baru | Operator |
| GET | `/items/{id}` | Detail item | Viewer |
| PUT | `/items/{id}` | Update item lengkap | Operator |
| PATCH | `/items/{id}` | Update item parsial | Operator |
| DELETE | `/items/{id}` | Hapus item | Manager |

**Buat Item:**

```bash
curl -X POST http://localhost:8080/api/v1/items \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "sku": "LAP001",
    "name": "Laptop 14 inch",
    "description": "Laptop kantor Intel i7",
    "unit_price_amount": 15000000,
    "currency": "IDR",
    "stock_quantity": 50,
    "min_stock_quantity": 10
  }'
```

**List Item:**

```bash
curl "http://localhost:8080/api/v1/items?page=1&page_size=20&search=laptop"
```

**Error Response:**

```json
{
  "type": "about:blank",
  "title": "Validation Error",
  "status": 422,
  "detail": "Name cannot be empty"
}
```

---

Dokumentasi OpenAPI lengkap: [`docs/api.yaml`](docs/api.yaml).
