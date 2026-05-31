#include <gtest/gtest.h>
#include "core/value_objects/quantity.hpp"

namespace inventory::test {

class TestQuantity : public ::testing::Test {
};

TEST_F(TestQuantity, CreateWithValidValue)
{
    auto result = Quantity::create(100, "pcs");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(100, result->value());
    EXPECT_EQ("pcs", result->unit());
}

TEST_F(TestQuantity, CreateWithDefaultUnit)
{
    auto result = Quantity::create(50);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(50, result->value());
    EXPECT_EQ("pcs", result->unit());
}

TEST_F(TestQuantity, CreateWithZero)
{
    auto result = Quantity::create(0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(0, result->value());
}

TEST_F(TestQuantity, CreateWithNegativeValueFails)
{
    auto result = Quantity::create(-10, "pcs");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestQuantity, Add)
{
    auto a = Quantity::create(30, "pcs").value();
    auto b = Quantity::create(20, "pcs").value();
    auto result = a.add(b);
    EXPECT_EQ(50, result.value());
}

TEST_F(TestQuantity, AddZero)
{
    auto a = Quantity::create(10, "pcs").value();
    auto b = Quantity::create(0, "pcs").value();
    auto result = a.add(b);
    EXPECT_EQ(10, result.value());
}

TEST_F(TestQuantity, SubtractNormal)
{
    auto a = Quantity::create(50, "pcs").value();
    auto b = Quantity::create(20, "pcs").value();
    auto result = a.subtract(b);
    EXPECT_EQ(30, result.value());
}

TEST_F(TestQuantity, SubtractResultClampsAtZero)
{
    auto a = Quantity::create(10, "pcs").value();
    auto b = Quantity::create(30, "pcs").value();
    auto result = a.subtract(b);
    EXPECT_EQ(0, result.value());
}

TEST_F(TestQuantity, SubtractExactAmount)
{
    auto a = Quantity::create(10, "pcs").value();
    auto b = Quantity::create(10, "pcs").value();
    auto result = a.subtract(b);
    EXPECT_EQ(0, result.value());
}

TEST_F(TestQuantity, IsEmptyTrue)
{
    auto q = Quantity::create(0).value();
    EXPECT_TRUE(q.is_empty());
}

TEST_F(TestQuantity, IsEmptyFalse)
{
    auto q = Quantity::create(1).value();
    EXPECT_FALSE(q.is_empty());
}

TEST_F(TestQuantity, LessThanOperator)
{
    auto small = Quantity::create(5, "pcs").value();
    auto large = Quantity::create(10, "pcs").value();
    EXPECT_TRUE(small < large);
    EXPECT_FALSE(large < small);
}

TEST_F(TestQuantity, GreaterThanOperator)
{
    auto small = Quantity::create(5, "pcs").value();
    auto large = Quantity::create(10, "pcs").value();
    EXPECT_TRUE(large > small);
    EXPECT_FALSE(small > large);
}

TEST_F(TestQuantity, EqualityOperator)
{
    auto a = Quantity::create(7, "pcs").value();
    auto b = Quantity::create(7, "pcs").value();
    EXPECT_TRUE(a == b);
}

TEST_F(TestQuantity, InequalityOperator)
{
    auto a = Quantity::create(7, "pcs").value();
    auto b = Quantity::create(8, "pcs").value();
    EXPECT_FALSE(a == b);
}

TEST_F(TestQuantity, EquivalentValues_NotEqualIfDifferentUnits)
{
    auto a = Quantity::create(5, "pcs").value();
    auto b = Quantity::create(5, "kg").value();
    EXPECT_FALSE(a == b);
}

} // namespace inventory::test
