# inventory_management_system — PT. Maju Bersama

Sistem manajemen inventaris gudang berbasis C++20 dengan PostgreSQL dan REST API.

## Stack

| Komponen | Teknologi |
|----------|-----------|
| Bahasa | C++20 |
| Build | CMake 3.27+ / Ninja |
| Compiler | GCC 13+ |
| Database | PostgreSQL 16 + libpqxx |
| HTTP | Drogon (async) |
| Test | Google Test + Google Mock |

## Prasyarat

- GCC 13+
- CMake 3.27+
- Ninja
- PostgreSQL 16
- libpqxx 7.x
- Drogon
- spdlog
- fmt
- nlohmann-json
- jwt-cpp
- OpenSSL

## Build & Run

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
./inventory_management_system
```

## Testing

```bash
cmake --build build --target test
# atau
cd build && ctest --output-on-failure
```

## Konfigurasi

Salin `config/config.yaml.example` ke `config/config.yaml` dan sesuaikan parameter.

## Endpoints

| Method | Path | Deskripsi |
|--------|------|-----------|
| POST   | /api/v1/auth/login | Login |
| POST   | /api/v1/auth/refresh | Refresh token |
| GET    | /api/v1/items | List item |
| POST   | /api/v1/items | Buat item |
| GET    | /api/v1/items/{id} | Detail item |
| PUT    | /api/v1/items/{id} | Update item |
| DELETE | /api/v1/items/{id} | Hapus item |
| PATCH  | /api/v1/items/{id} | Patch item |
