#include "group_translator.hpp"

// Matter components
// #include <lib/support/UnitTestContext.h>
// #include <lib/support/UnitTestRegistration.h>

// Third party library
#include <gtest/gtest.h>
// mocks
#include "MockMatterDataStorage.hpp"

using namespace unify::matter_bridge;
#define TEST_LOG_TAG "TestGroupTranslator"

namespace chip {
namespace app {
namespace TestPath {

TEST(TestGroup, TestGroupTranslator)
{
    unify::matter_bridge::Test::MockMatterDataStorage mock_data_storage;
    group_translator test_group_translator(mock_data_storage);
    group_translator::matter_group test_matter_group = { 1, 1 };

    EXPECT_TRUE(test_group_translator.add_matter_group(test_matter_group));
    EXPECT_EQ(test_group_translator.get_unify_group(test_matter_group),1);
}

TEST(TestGroup, TestGroupTranslatorUnifyGroupAdd)
{
    unify::matter_bridge::Test::MockMatterDataStorage mock_data_storage;
    group_translator test_group_translator(mock_data_storage);
    // Assigned unify group 
    test_group_translator.register_unify_group(1);
    // Assign matter group
    group_translator::matter_group test_matter_group = { 1, 1 };
    EXPECT_TRUE(test_group_translator.add_matter_group(test_matter_group));
    // Check the unify group incremented 
    EXPECT_EQ(test_group_translator.get_unify_group(test_matter_group),2);
}
}
}
}