/// @file test_omniflow_extra.cpp
/// @brief Additional OmniFlow tests — rename, conditions, extension filter, size filter.

#include <gtest/gtest.h>
#include "fo/core/omniflow_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>

using namespace fo::core;

class OmniFlowExtraTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir_ = std::filesystem::temp_directory_path() / ("fo_flowx_" + unique_id);
        std::filesystem::create_directories(test_dir_);
        dest_dir_ = test_dir_ / "dest";
        std::filesystem::create_directories(dest_dir_);
    }
    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }
    void create_file(const std::filesystem::path& p, const std::string& content = "test") {
        std::ofstream(p) << content;
    }
    std::filesystem::path test_dir_;
    std::filesystem::path dest_dir_;
};

TEST_F(OmniFlowExtraTest, ActionRenameFile) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto src = test_dir_ / "original.txt";
    create_file(src, "content");

    Workflow wf;
    wf.id = "rename-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"a1", "Action.Rename", FlowNodeType::Action, {{"pattern", "renamed.txt"}}});
    wf.connections.push_back({"t1", "out", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("rename-test", src));
    EXPECT_TRUE(std::filesystem::exists(test_dir_ / "renamed.txt"));
    EXPECT_FALSE(std::filesystem::exists(src));
}

TEST_F(OmniFlowExtraTest, ConditionAlwaysTrue) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto f = test_dir_ / "any.txt";
    create_file(f);

    Workflow wf;
    wf.id = "always-true-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"c1", "Condition.AlwaysTrue", FlowNodeType::Condition, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "c1", "in"});
    wf.connections.push_back({"c1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("always-true-test", f));
    EXPECT_TRUE(std::filesystem::exists(dest_dir_ / "any.txt"));
}

TEST_F(OmniFlowExtraTest, ConditionAlwaysFalse) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto f = test_dir_ / "blocked.txt";
    create_file(f);

    Workflow wf;
    wf.id = "always-false-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"c1", "Condition.AlwaysFalse", FlowNodeType::Condition, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "c1", "in"});
    wf.connections.push_back({"c1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("always-false-test", f));
    EXPECT_FALSE(std::filesystem::exists(dest_dir_ / "blocked.txt"));
}

TEST_F(OmniFlowExtraTest, WorkflowWithNoTriggersReturnsFalse) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    Workflow wf;
    wf.id = "no-trigger-test";
    wf.is_active = true;
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});

    engine->register_workflow(wf);
    EXPECT_FALSE(engine->execute_workflow("no-trigger-test", test_dir_ / "file.txt"));
}

TEST_F(OmniFlowExtraTest, FilterExtensionPasses) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto doc = test_dir_ / "readme.doc";
    create_file(doc);

    Workflow wf;
    wf.id = "ext-filter-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.Extension", FlowNodeType::Filter, {{"ext", ".doc"}}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("ext-filter-test", doc));
    EXPECT_TRUE(std::filesystem::exists(dest_dir_ / "readme.doc"));
}

TEST_F(OmniFlowExtraTest, FilterExtensionRejects) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto txt = test_dir_ / "notes.txt";
    create_file(txt);

    Workflow wf;
    wf.id = "ext-reject-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.Extension", FlowNodeType::Filter, {{"ext", ".doc"}}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("ext-reject-test", txt));
    EXPECT_FALSE(std::filesystem::exists(dest_dir_ / "notes.txt"));
}

TEST_F(OmniFlowExtraTest, FilterIsLargerPasses) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto big = test_dir_ / "bigfile.bin";
    create_file(big, std::string(2048, 'X'));

    Workflow wf;
    wf.id = "larger-filter-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsLarger", FlowNodeType::Filter, {{"min_bytes", "1024"}}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("larger-filter-test", big));
    EXPECT_TRUE(std::filesystem::exists(dest_dir_ / "bigfile.bin"));
}

TEST_F(OmniFlowExtraTest, FilterIsLargerRejectsSmallFile) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto small = test_dir_ / "small.txt";
    create_file(small, "hi");

    Workflow wf;
    wf.id = "larger-reject-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsLarger", FlowNodeType::Filter, {{"min_bytes", "1024"}}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("larger-reject-test", small));
    EXPECT_FALSE(std::filesystem::exists(dest_dir_ / "small.txt"));
}
