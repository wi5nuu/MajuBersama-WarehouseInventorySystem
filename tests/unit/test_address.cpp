#include <gtest/gtest.h>
#include "core/value_objects/address.hpp"

namespace inventory::test {

class TestAddress : public ::testing::Test {
};

TEST_F(TestAddress, CreateWithValidFields)
{
    auto result = Address::create("Jl. Merdeka No.1", "Jakarta", "DKI Jakarta", "10110", "Indonesia");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("Jl. Merdeka No.1", result->street());
    EXPECT_EQ("Jakarta", result->city());
    EXPECT_EQ("DKI Jakarta", result->province());
    EXPECT_EQ("10110", result->postal_code());
    EXPECT_EQ("Indonesia", result->country());
}

TEST_F(TestAddress, CreateWithDefaultCountry)
{
    auto result = Address::create("Jl. Sudirman", "Bandung", "Jawa Barat", "40123");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("Indonesia", result->country());
}

TEST_F(TestAddress, CreateWithEmptyStreetFails)
{
    auto result = Address::create("", "Jakarta", "DKI Jakarta", "10110");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestAddress, CreateWithEmptyCityFails)
{
    auto result = Address::create("Jl. Test", "", "Jawa Barat", "40123");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestAddress, CreateWithEmptyPostalCodeFails)
{
    auto result = Address::create("Jl. Test", "Bandung", "Jawa Barat", "");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestAddress, CreateWithEmptyProvinceSucceeds)
{
    auto result = Address::create("Jl. Test", "Surabaya", "", "60111");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->province().empty());
}

TEST_F(TestAddress, FullAddressFormatting)
{
    auto addr = Address::create("Jl. Merdeka No.1", "Jakarta Pusat", "DKI Jakarta", "10110").value();
    std::string expected = "Jl. Merdeka No.1, Jakarta Pusat, DKI Jakarta 10110, Indonesia";
    EXPECT_EQ(expected, addr.full_address());
}

TEST_F(TestAddress, FullAddressWithEmptyProvince)
{
    auto addr = Address::create("Jl. Test", "Surabaya", "", "60111").value();
    std::string expected = "Jl. Test, Surabaya,  60111, Indonesia";
    EXPECT_EQ(expected, addr.full_address());
}

TEST_F(TestAddress, EqualityOperator)
{
    auto a = Address::create("Jl. A", "City", "Prov", "12345").value();
    auto b = Address::create("Jl. A", "City", "Prov", "12345").value();
    EXPECT_TRUE(a == b);
}

TEST_F(TestAddress, InequalityOperator)
{
    auto a = Address::create("Jl. A", "City", "Prov", "12345").value();
    auto b = Address::create("Jl. B", "City", "Prov", "12345").value();
    EXPECT_FALSE(a == b);
}

} // namespace inventory::test
