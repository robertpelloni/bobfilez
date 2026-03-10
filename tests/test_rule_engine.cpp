#include <gtest/gtest.h>
#include "fo/core/rule_engine.hpp"
#include <filesystem>
#include <chrono>

using namespace fo::core;

class RuleEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<RuleEngine>();
    }

    FileInfo create_test_file(const std::string& path_str) {
        FileInfo file;
        file.uri = path_str;
        file.size = 1024;
        file.mtime = std::chrono::file_clock::now();
        file.is_dir = false;
        return file;
    }

    std::unique_ptr<RuleEngine> engine;
};

TEST_F(RuleEngineTest, NoRulesReturnsNullopt) {
    FileInfo file = create_test_file("/home/user/photo.jpg");
    auto result = engine->apply_rules(file, {});
    EXPECT_FALSE(result.has_value());
}

TEST_F(RuleEngineTest, EmptyFilterMatchesAllFiles) {
    OrganizationRule rule;
    rule.name = "all_files";
    rule.filter_tag = "";
    rule.destination_template = "/archive/";
    engine->add_rule(rule);

    FileInfo file = create_test_file("/home/user/document.pdf");
    auto result = engine->apply_rules(file, {});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->filename().string(), "document.pdf");
}

TEST_F(RuleEngineTest, FilterTagMatchesCorrectly) {
    OrganizationRule rule;
    rule.name = "photos_only";
    rule.filter_tag = "photo";
    rule.destination_template = "/photos/";
    engine->add_rule(rule);

    FileInfo file = create_test_file("/home/user/image.jpg");

    auto result_no_tag = engine->apply_rules(file, {});
    EXPECT_FALSE(result_no_tag.has_value());

    auto result_wrong_tag = engine->apply_rules(file, {"document", "video"});
    EXPECT_FALSE(result_wrong_tag.has_value());

    auto result_correct_tag = engine->apply_rules(file, {"photo"});
    ASSERT_TRUE(result_correct_tag.has_value());
}

TEST_F(RuleEngineTest, FirstMatchingRuleWins) {
    OrganizationRule rule1;
    rule1.name = "first";
    rule1.filter_tag = "";
    rule1.destination_template = "/first/";
    engine->add_rule(rule1);

    OrganizationRule rule2;
    rule2.name = "second";
    rule2.filter_tag = "";
    rule2.destination_template = "/second/";
    engine->add_rule(rule2);

    FileInfo file = create_test_file("/home/user/file.txt");
    auto result = engine->apply_rules(file, {});

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->string().find("first") != std::string::npos);
}

TEST_F(RuleEngineTest, ExpandTemplateWithName) {
    OrganizationRule rule;
    rule.name = "name_test";
    rule.filter_tag = "";
    rule.destination_template = "/backup/{name}.bak";
    engine->add_rule(rule);

    FileInfo file = create_test_file("/home/user/document.pdf");
    auto result = engine->apply_rules(file, {});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->filename().string(), "document.bak");
}

TEST_F(RuleEngineTest, ExpandTemplateWithExt) {
    OrganizationRule rule;
    rule.name = "ext_test";
    rule.filter_tag = "";
    rule.destination_template = "/sorted/{ext}/file.txt";
    engine->add_rule(rule);

    FileInfo file = create_test_file("/home/user/image.jpg");
    auto result = engine->apply_rules(file, {});

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->string().find("jpg") != std::string::npos);
}

TEST_F(RuleEngineTest, ExpandTemplateWithYearMonthDay) {
    OrganizationRule rule;
    rule.name = "date_test";
    rule.filter_tag = "";
    rule.destination_template = "/archive/{year}/{month}/{day}/";
    engine->add_rule(rule);

    FileInfo file = create_test_file("/home/user/photo.jpg");
    auto result = engine->apply_rules(file, {});

    ASSERT_TRUE(result.has_value());
    std::string path_str = result->string();
    EXPECT_TRUE(path_str.find("{year}") == std::string::npos);
    EXPECT_TRUE(path_str.find("{month}") == std::string::npos);
    EXPECT_TRUE(path_str.find("{day}") == std::string::npos);
}

TEST_F(RuleEngineTest, ExpandTemplateWithParent) {
    OrganizationRule rule;
    rule.name = "parent_test";
    rule.filter_tag = "";
    rule.destination_template = "/backup{parent}/file.txt";
    engine->add_rule(rule);

#ifdef _WIN32
    FileInfo file = create_test_file("C:\\Users\\test\\documents\\report.pdf");
#else
    FileInfo file = create_test_file("/home/user/documents/report.pdf");
#endif
    auto result = engine->apply_rules(file, {});

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->string().find("documents") != std::string::npos);
}

TEST_F(RuleEngineTest, DestinationWithTrailingSeparatorAppendsFilename) {
    OrganizationRule rule;
    rule.name = "trailing_sep";
    rule.filter_tag = "";
    rule.destination_template = "/destination/";
    engine->add_rule(rule);

    FileInfo file = create_test_file("/source/myfile.txt");
    auto result = engine->apply_rules(file, {});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->filename().string(), "myfile.txt");
}

TEST_F(RuleEngineTest, DestinationWithoutTrailingSeparatorUsesAsIs) {
    OrganizationRule rule;
    rule.name = "no_trailing_sep";
    rule.filter_tag = "";
    rule.destination_template = "/destination/renamed.txt";
    engine->add_rule(rule);

    FileInfo file = create_test_file("/source/original.txt");
    auto result = engine->apply_rules(file, {});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->filename().string(), "renamed.txt");
}

TEST_F(RuleEngineTest, MultipleTagsMatchSingleFilter) {
    OrganizationRule rule;
    rule.name = "multi_tag";
    rule.filter_tag = "important";
    rule.destination_template = "/important/";
    engine->add_rule(rule);

    FileInfo file = create_test_file("/home/user/doc.txt");
    auto result = engine->apply_rules(file, {"work", "important", "2024"});

    ASSERT_TRUE(result.has_value());
}

TEST_F(RuleEngineTest, ComplexTemplateExpansion) {
    OrganizationRule rule;
    rule.name = "complex";
    rule.filter_tag = "";
    rule.destination_template = "/photos/{year}/{month}/{name}.{ext}";
    engine->add_rule(rule);

    FileInfo file = create_test_file("/camera/IMG_001.jpg");
    auto result = engine->apply_rules(file, {});

    ASSERT_TRUE(result.has_value());
    std::string path_str = result->string();
    EXPECT_TRUE(path_str.find("IMG_001") != std::string::npos);
    EXPECT_TRUE(path_str.find("jpg") != std::string::npos);
    EXPECT_TRUE(path_str.find("{") == std::string::npos);
}
