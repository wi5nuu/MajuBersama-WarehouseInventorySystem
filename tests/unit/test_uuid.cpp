#include <gtest/gtest.h>
#include <set>
#include <string>
#include "common/utils/uuid.hpp"

namespace inventory::test {

class TestUuid : public ::testing::Test {
};

TEST_F(TestUuid, GenerateReturnsNonEmptyUuid)
{
    auto uuid = utils::Uuid::generate();
    EXPECT_TRUE(uuid.is_valid());
}

TEST_F(TestUuid, GenerateReturnsDifferentUuids)
{
    auto a = utils::Uuid::generate();
    auto b = utils::Uuid::generate();
    EXPECT_NE(a, b);
}

TEST_F(TestUuid, GenerateReturnsUniqueValuesInBatch)
{
    std::set<utils::Uuid> uuids;
    for (int i = 0; i < 100; ++i) {
        uuids.insert(utils::Uuid::generate());
    }
    EXPECT_EQ(100, uuids.size());
}

TEST_F(TestUuid, ToStringReturnsCorrectFormat)
{
    auto uuid = utils::Uuid::generate();
    auto str = uuid.to_string();
    EXPECT_EQ(36, str.size());
    EXPECT_EQ('-', str[8]);
    EXPECT_EQ('-', str[13]);
    EXPECT_EQ('-', str[18]);
    EXPECT_EQ('-', str[23]);
}

TEST_F(TestUuid, ToStringContainsOnlyHexAndDashes)
{
    auto uuid = utils::Uuid::generate();
    auto str = uuid.to_string();
    for (size_t i = 0; i < str.size(); ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            EXPECT_EQ('-', str[i]);
        } else {
            EXPECT_TRUE(std::isxdigit(static_cast<unsigned char>(str[i])));
        }
    }
}

TEST_F(TestUuid, FromStringRoundTrip)
{
    auto original = utils::Uuid::generate();
    auto str = original.to_string();
    auto restored = utils::Uuid::from_string(str);
    EXPECT_TRUE(restored.is_valid());
    EXPECT_EQ(original, restored);
}

TEST_F(TestUuid, FromStringWithValidUuidString)
{
    auto uuid = utils::Uuid::from_string("550e8400-e29b-41d4-a716-446655440000");
    EXPECT_TRUE(uuid.is_valid());
    EXPECT_EQ("550e8400-e29b-41d4-a716-446655440000", uuid.to_string());
}

TEST_F(TestUuid, FromStringWithInvalidFormatReturnsInvalid)
{
    auto uuid = utils::Uuid::from_string("not-a-uuid");
    EXPECT_FALSE(uuid.is_valid());
}

TEST_F(TestUuid, FromStringWithEmptyStringReturnsInvalid)
{
    auto uuid = utils::Uuid::from_string("");
    EXPECT_FALSE(uuid.is_valid());
}

TEST_F(TestUuid, FromStringWithPartialHexReturnsInvalid)
{
    auto uuid = utils::Uuid::from_string("550e8400-e29b-41d4");
    EXPECT_FALSE(uuid.is_valid());
}

TEST_F(TestUuid, DefaultConstructedUuidIsInvalid)
{
    utils::Uuid uuid;
    EXPECT_FALSE(uuid.is_valid());
}

TEST_F(TestUuid, VersionBitsSetCorrectly)
{
    auto uuid = utils::Uuid::generate();
    auto str = uuid.to_string();
    EXPECT_EQ('4', str[14]);
}

TEST_F(TestUuid, VariantBitsSetCorrectly)
{
    auto uuid = utils::Uuid::generate();
    auto str = uuid.to_string();
    char c = str[19];
    EXPECT_TRUE(c == '8' || c == '9' || c == 'a' || c == 'b');
}

} // namespace inventory::test
