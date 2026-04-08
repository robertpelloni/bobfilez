#include <gtest/gtest.h>
#include "fo/core/lint_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <fstream>
#include <filesystem>

using namespace fo::core;

class StdLinterTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        
        test_dir = std::filesystem::temp_directory_path() / "fo_test_linter";
        std::filesystem::create_directories(test_dir);
    }
    
    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }
    
    std::filesystem::path test_dir;
};

TEST_F(StdLinterTest, LinterExists) {
    auto linter = Registry<ILinter>::instance().create("std");
    ASSERT_NE(linter, nullptr);
    EXPECT_EQ(linter->name(), "std");
}

TEST_F(StdLinterTest, DetectsEmptyFile) {
    auto linter = Registry<ILinter>::instance().create("std");
    auto empty_file = test_dir / "empty.txt";
    std::ofstream(empty_file).close(); // Create empty file
    
    auto results = linter->lint({test_dir});
    
    bool found = false;
    for (const auto& r : results) {
        if (r.path == empty_file && r.type == LintType::EmptyFile) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(StdLinterTest, DetectsEmptyDirectory) {
    auto linter = Registry<ILinter>::instance().create("std");
    auto empty_subdir = test_dir / "empty_subdir";
    std::filesystem::create_directories(empty_subdir);
    
    auto results = linter->lint({test_dir});
    
    bool found = false;
    for (const auto& r : results) {
        if (r.path == empty_subdir && r.type == LintType::EmptyDirectory) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(StdLinterTest, DetectsTempFile) {
    auto linter = Registry<ILinter>::instance().create("std");
    auto temp_file = test_dir / "doc.bak";
    std::ofstream(temp_file) << "content"; 
    
    auto results = linter->lint({test_dir});
    
    bool found = false;
    for (const auto& r : results) {
        if (r.path == temp_file && r.type == LintType::TemporaryFile) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(StdLinterTest, DetectsHiddenFile) {
    auto linter = Registry<ILinter>::instance().create("std");
    auto hidden = test_dir / ".hidden_config";
    std::ofstream(hidden) << "config data";
    
    auto results = linter->lint({test_dir});
    
    bool found = false;
    for (const auto& r : results) {
        if (r.path == hidden && r.type == LintType::HiddenFile) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(StdLinterTest, DetectsDuplicateNames) {
    auto linter = Registry<ILinter>::instance().create("std");
    auto sub_a = test_dir / "sub_a";
    auto sub_b = test_dir / "sub_b";
    std::filesystem::create_directories(sub_a);
    std::filesystem::create_directories(sub_b);
    std::ofstream(sub_a / "readme.txt") << "aaa";
    std::ofstream(sub_b / "readme.txt") << "bbb";
    
    auto results = linter->lint({test_dir});
    
    int dup_count = 0;
    for (const auto& r : results) {
        if (r.type == LintType::DuplicateName) {
            dup_count++;
        }
    }
    EXPECT_GE(dup_count, 2); // Both copies should be flagged
}

TEST_F(StdLinterTest, DetectsThumbsDb) {
    auto linter = Registry<ILinter>::instance().create("std");
    auto thumbs = test_dir / "Thumbs.db";
    std::ofstream(thumbs) << "binary";
    
    auto results = linter->lint({test_dir});
    
    bool found = false;
    for (const auto& r : results) {
        if (r.path == thumbs && r.type == LintType::TemporaryFile) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}
