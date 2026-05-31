# ARCHITECTURE.md

## Arsitektur: Clean Architecture + Domain-Driven Design

```
api/       → HTTP controllers, DTOs, middleware, validators
core/      → Domain entities, value objects, repository interfaces, use cases
infrastructure/ → Database, cache, messaging, repository implementations
common/    → Result<T,E>, logger, config, utilities
```

### Lapisan

1. **Domain (core)** — Business rules murni, tanpa dependensi framework
2. **Application (use cases)** — Orchestrasi business rules
3. **Infrastructure** — Implementasi konkret (DB, HTTP client, cache)
4. **Presentation (api)** — HTTP handlers, serialization

### Aliran Data

```
Client → Controller → UseCase → Repository (interface)
                                    ↑
                           Repository (infrastructure)
                                    ↓
                                Database
```

### Error Handling

Semua fungsi mengembalikan `Result<T, Error>` — tidak ada exception.

### Autentikasi

JWT access token (15 menit) + refresh token (7 hari) dengan RBAC.
