#include <gtest/gtest.h>
#include "core/value_objects/money.hpp"

namespace inventory::test {

class TestMoney : public ::testing::Test {
};

TEST_F(TestMoney, CreateWithValidAmount)
{
    auto result = Money::create(15000, "IDR");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(15000, result->amount());
    EXPECT_EQ("IDR", result->currency());
}

TEST_F(TestMoney, CreateWithDefaultCurrency)
{
    auto result = Money::create(5000);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(5000, result->amount());
    EXPECT_EQ("IDR", result->currency());
}

TEST_F(TestMoney, CreateWithZeroAmount)
{
    auto result = Money::create(0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(0, result->amount());
}

TEST_F(TestMoney, CreateWithNegativeAmountFails)
{
    auto result = Money::create(-100, "IDR");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestMoney, CreateWithInvalidCurrencyLengthFails)
{
    auto result = Money::create(100, "US");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestMoney, CreateWithEmptyCurrencyFails)
{
    auto result = Money::create(100, "");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(ErrorCode::kValidationError, result.error().code());
}

TEST_F(TestMoney, Add)
{
    auto a = Money::create(10000, "IDR").value();
    auto b = Money::create(2500, "IDR").value();
    auto result = a.add(b);
    EXPECT_EQ(12500, result.amount());
}

TEST_F(TestMoney, Subtract)
{
    auto a = Money::create(10000, "IDR").value();
    auto b = Money::create(3000, "IDR").value();
    auto result = a.subtract(b);
    EXPECT_EQ(7000, result.amount());
}

TEST_F(TestMoney, SubtractResultCanBeNegative)
{
    auto a = Money::create(5000, "IDR").value();
    auto b = Money::create(8000, "IDR").value();
    auto result = a.subtract(b);
    EXPECT_EQ(-3000, result.amount());
}

TEST_F(TestMoney, Multiply)
{
    auto m = Money::create(10000, "IDR").value();
    auto result = m.multiply(2.5);
    EXPECT_EQ(25000, result.amount());
}

TEST_F(TestMoney, MultiplyByZero)
{
    auto m = Money::create(5000, "IDR").value();
    auto result = m.multiply(0.0);
    EXPECT_EQ(0, result.amount());
}

TEST_F(TestMoney, MultiplyByFraction)
{
    auto m = Money::create(100, "IDR").value();
    auto result = m.multiply(0.3);
    EXPECT_EQ(30, result.amount());
}

TEST_F(TestMoney, ToStringFormat)
{
    auto m = Money::create(15000, "IDR").value();
    EXPECT_EQ("150.00 IDR", m.to_string());
}

TEST_F(TestMoney, ToStringWithMinorUnits)
{
    auto m = Money::create(15099, "IDR").value();
    EXPECT_EQ("150.99 IDR", m.to_string());
}

TEST_F(TestMoney, ToStringZeroAmount)
{
    auto m = Money::create(0, "IDR").value();
    EXPECT_EQ("0.00 IDR", m.to_string());
}

TEST_F(TestMoney, ToStringOtherCurrency)
{
    auto m = Money::create(5000, "USD").value();
    EXPECT_EQ("50.00 USD", m.to_string());
}

TEST_F(TestMoney, EqualityOperator)
{
    auto a = Money::create(1000, "IDR").value();
    auto b = Money::create(1000, "IDR").value();
    EXPECT_TRUE(a == b);
}

TEST_F(TestMoney, InequalityOperator)
{
    auto a = Money::create(1000, "IDR").value();
    auto b = Money::create(2000, "IDR").value();
    EXPECT_FALSE(a == b);
}

} // namespace inventory::test
