-- Test data for inventory_management_system integration tests
-- This script inserts sample data into all tables
INSERT INTO users (id, username, email, password_hash, role, is_active)
VALUES
    ('a0000000-0000-0000-0000-000000000001', 'testadmin', 'admin@test.com', 'hash123', 'admin', true),
    ('a0000000-0000-0000-0000-000000000002', 'testops', 'ops@test.com', 'hash456', 'operator', true);

INSERT INTO categories (id, name, description, is_active)
VALUES
    ('b0000000-0000-0000-0000-000000000001', 'Elektronik', 'Electronic items', true),
    ('b0000000-0000-0000-0000-000000000002', 'Furniture', 'Furniture items', true);

INSERT INTO suppliers (id, code, name, contact_person, phone, email, city, country, is_active)
VALUES
    ('c0000000-0000-0000-0000-000000000001', 'SUP001', 'PT Elektronik Maju', 'Budi', '021-1234', 'budi@elektronik.id', 'Jakarta', 'Indonesia', true),
    ('c0000000-0000-0000-0000-000000000002', 'SUP002', 'PT Furniture Indo', 'Siti', '021-5678', 'siti@furniture.id', 'Bandung', 'Indonesia', true);

INSERT INTO warehouses (id, code, name, city, country, is_active)
VALUES
    ('d0000000-0000-0000-0000-000000000001', 'WH01', 'Gudang Utama', 'Jakarta', 'Indonesia', true),
    ('d0000000-0000-0000-0000-000000000002', 'WH02', 'Gudang Cabang', 'Surabaya', 'Indonesia', true);

INSERT INTO items (id, sku, name, description, category_id, supplier_id, unit_price_amount, unit_price_currency, stock_quantity, stock_unit, min_stock_quantity, status)
VALUES
    ('e0000000-0000-0000-0000-000000000001', 'SKU001', 'Laptop', 'Laptop 14 inch', 'b0000000-0000-0000-0000-000000000001', 'c0000000-0000-0000-0000-000000000001', 15000000, 'IDR', 50, 'pcs', 10, 'active'),
    ('e0000000-0000-0000-0000-000000000002', 'SKU002', 'Meja', 'Meja kantor', 'b0000000-0000-0000-0000-000000000002', 'c0000000-0000-0000-0000-000000000002', 2000000, 'IDR', 5, 'pcs', 10, 'active');
