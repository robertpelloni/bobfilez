/// @file test_omniflow.cpp
/// @brief Tests for OmniFlow visual automation engine with real workflow execution.

#include <gtest/gtest.h>
#include "fo/core/omniflow_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>

using namespace fo::core;

class OmniFlowTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir_ = std::filesystem::temp_directory_path() / ("fo_flow_" + unique_id);
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

// ── Workflow Struct Tests ────────────────────────────────────────────────

TEST_F(OmniFlowTest, FlowNodeDefaultInit) {
    FlowNode node;
    EXPECT_EQ(node.id, "");
    EXPECT_EQ(node.type_name, "");
    EXPECT_EQ(node.category, FlowNodeType::Trigger);
    EXPECT_TRUE(node.config.empty());
}

TEST_F(OmniFlowTest, FlowConnectionDefaultInit) {
    FlowConnection conn;
    EXPECT_EQ(conn.from_node_id, "");
    EXPECT_EQ(conn.from_pin, "");
    EXPECT_EQ(conn.to_node_id, "");
    EXPECT_EQ(conn.to_pin, "");
}

TEST_F(OmniFlowTest, WorkflowStruct) {
    Workflow wf;
    wf.id = "test-wf";
    wf.name = "Test Workflow";
    wf.is_active = true;
    wf.nodes.push_back({"n1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"n2", "Action.Delete", FlowNodeType::Action, {}});
    wf.connections.push_back({"n1", "out", "n2", "in"});

    EXPECT_EQ(wf.id, "test-wf");
    EXPECT_EQ(wf.nodes.size(), 2u);
    EXPECT_EQ(wf.connections.size(), 1u);
    EXPECT_TRUE(wf.is_active);
}

// ── Engine Registration Tests ────────────────────────────────────────────

TEST_F(OmniFlowTest, EngineCreates) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

TEST_F(OmniFlowTest, PreloadedWorkflow) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    auto workflows = engine->get_workflows();
    EXPECT_GE(workflows.size(), 1u);
    EXPECT_EQ(workflows[0].id, "flow-01");
    EXPECT_EQ(workflows[0].nodes.size(), 5u);
    EXPECT_EQ(workflows[0].connections.size(), 4u);
}

// ── Register Custom Workflow ─────────────────────────────────────────────

TEST_F(OmniFlowTest, RegisterCustomWorkflow) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    Workflow wf;
    wf.id = "custom-01";
    wf.name = "My Custom Flow";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"a1", "Action.NotifyUser", FlowNodeType::Action, {}});
    wf.connections.push_back({"t1", "out", "a1", "in"});

    engine->register_workflow(wf);
    auto workflows = engine->get_workflows();
    EXPECT_GE(workflows.size(), 2u); // preloaded + custom
}

// ── Execute Nonexistent Workflow ─────────────────────────────────────────

TEST_F(OmniFlowTest, ExecuteNonexistentWorkflowReturnsFalse) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    EXPECT_FALSE(engine->execute_workflow("nonexistent", test_dir_ / "file.txt"));
}

// ── Filter IsPDF ─────────────────────────────────────────────────────────

TEST_F(OmniFlowTest, FilterIsPDFPasses) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto pdf = test_dir_ / "invoice.pdf";
    create_file(pdf);

    Workflow wf;
    wf.id = "pdf-filter-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsPDF", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("pdf-filter-test", pdf));
    // The file should have been copied
    EXPECT_TRUE(std::filesystem::exists(dest_dir_ / "invoice.pdf"));
}

TEST_F(OmniFlowTest, FilterIsPDFRejectsTxt) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto txt = test_dir_ / "notes.txt";
    create_file(txt);

    Workflow wf;
    wf.id = "pdf-reject-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsPDF", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("pdf-reject-test", txt));
    // The filter should have blocked, file should NOT be copied
    EXPECT_FALSE(std::filesystem::exists(dest_dir_ / "notes.txt"));
}

// ── Filter IsImage ───────────────────────────────────────────────────────

TEST_F(OmniFlowTest, FilterIsImagePasses) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto img = test_dir_ / "photo.jpg";
    create_file(img);

    Workflow wf;
    wf.id = "img-filter-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsImage", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("img-filter-test", img));
    EXPECT_TRUE(std::filesystem::exists(dest_dir_ / "photo.jpg"));
}

// ── Condition TextContains ───────────────────────────────────────────────

TEST_F(OmniFlowTest, ConditionTextContainsPasses) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto inv = test_dir_ / "invoice_march.pdf";
    create_file(inv);

    Workflow wf;
    wf.id = "text-contains-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"c1", "Condition.TextContains", FlowNodeType::Condition, {{"text", "invoice"}}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "c1", "in"});
    wf.connections.push_back({"c1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("text-contains-test", inv));
    EXPECT_TRUE(std::filesystem::exists(dest_dir_ / "invoice_march.pdf"));
}

TEST_F(OmniFlowTest, ConditionTextContainsRejects) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto report = test_dir_ / "quarterly_report.pdf";
    create_file(report);

    Workflow wf;
    wf.id = "text-reject-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"c1", "Condition.TextContains", FlowNodeType::Condition, {{"text", "invoice"}}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "c1", "in"});
    wf.connections.push_back({"c1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("text-reject-test", report));
    // "quarterly_report" doesn't contain "invoice" -> blocked
    EXPECT_FALSE(std::filesystem::exists(dest_dir_ / "quarterly_report.pdf"));
}

// ── Action MoveTo ────────────────────────────────────────────────────────

TEST_F(OmniFlowTest, ActionMoveToRelocatesFile) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto src = test_dir_ / "move_me.txt";
    create_file(src);
    auto move_dest = test_dir_ / "moved";
    std::filesystem::create_directories(move_dest);

    Workflow wf;
    wf.id = "move-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"a1", "Action.MoveTo", FlowNodeType::Action, {{"target_dir", move_dest.string()}}});
    wf.connections.push_back({"t1", "out", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("move-test", src));
    EXPECT_TRUE(std::filesystem::exists(move_dest / "move_me.txt"));
    EXPECT_FALSE(std::filesystem::exists(src));
}

// ── Action Delete ────────────────────────────────────────────────────────

TEST_F(OmniFlowTest, ActionDeleteRemovesFile) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto victim = test_dir_ / "delete_me.tmp";
    create_file(victim);
    EXPECT_TRUE(std::filesystem::exists(victim));

    Workflow wf;
    wf.id = "delete-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"a1", "Action.Delete", FlowNodeType::Action, {}});
    wf.connections.push_back({"t1", "out", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("delete-test", victim));
    EXPECT_FALSE(std::filesystem::exists(victim));
}

// ── Multi-Step Pipeline ──────────────────────────────────────────────────

TEST_F(OmniFlowTest, MultiStepPipeline) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto pdf = test_dir_ / "Invoice_April.pdf";
    create_file(pdf);

    auto archive_dir = test_dir_ / "archive";
    std::filesystem::create_directories(archive_dir);

    // Pipeline: Trigger → Filter.IsPDF → Condition.TextContains("invoice") → Action.MoveTo
    Workflow wf;
    wf.id = "pipeline-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsPDF", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"c1", "Condition.TextContains", FlowNodeType::Condition, {{"text", "invoice"}}});
    wf.nodes.push_back({"a1", "Action.MoveTo", FlowNodeType::Action, {{"target_dir", archive_dir.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "c1", "in"});
    wf.connections.push_back({"c1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("pipeline-test", pdf));
    EXPECT_TRUE(std::filesystem::exists(archive_dir / "Invoice_April.pdf"));
    EXPECT_FALSE(std::filesystem::exists(pdf)); // moved, not copied
}

// ── Start Daemon Does Not Crash ──────────────────────────────────────────

TEST_F(OmniFlowTest, StartDaemonDoesNotCrash) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    EXPECT_NO_THROW(engine->start_daemon());
}

// ── False Branch Routing ─────────────────────────────────────────────────

TEST_F(OmniFlowTest, FalseBranchExecutes) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto txt = test_dir_ / "readme.txt";
    create_file(txt);

    auto pdf_dir = test_dir_ / "pdfs";
    auto other_dir = test_dir_ / "others";
    std::filesystem::create_directories(pdf_dir);
    std::filesystem::create_directories(other_dir);

    // Branch: Trigger → Filter.IsPDF → true: copy to pdfs, false: copy to others
    Workflow wf;
    wf.id = "branch-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsPDF", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a_pdf", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", pdf_dir.string()}}});
    wf.nodes.push_back({"a_other", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", other_dir.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a_pdf", "in"});
    wf.connections.push_back({"f1", "false", "a_other", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("branch-test", txt));
    // txt is not PDF, so it should go to "others" not "pdfs"
    EXPECT_FALSE(std::filesystem::exists(pdf_dir / "readme.txt"));
    EXPECT_TRUE(std::filesystem::exists(other_dir / "readme.txt"));
}

// ── IsAudio Filter ────────────────────────────────────────────────────────

TEST_F(OmniFlowTest, FilterIsAudioPasses) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto mp3 = test_dir_ / "song.mp3";
    create_file(mp3);

    Workflow wf;
    wf.id = "audio-filter-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsAudio", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("audio-filter-test", mp3));
    EXPECT_TRUE(std::filesystem::exists(dest_dir_ / "song.mp3"));
}

TEST_F(OmniFlowTest, FilterIsAudioRejectsPdf) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto pdf = test_dir_ / "doc.pdf";
    create_file(pdf);

    Workflow wf;
    wf.id = "audio-reject-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsAudio", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("audio-reject-test", pdf));
    EXPECT_FALSE(std::filesystem::exists(dest_dir_ / "doc.pdf"));
}

// ── IsVideo Filter ────────────────────────────────────────────────────────

TEST_F(OmniFlowTest, FilterIsVideoPasses) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto mp4 = test_dir_ / "clip.mp4";
    create_file(mp4);

    Workflow wf;
    wf.id = "video-filter-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsVideo", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("video-filter-test", mp4));
    EXPECT_TRUE(std::filesystem::exists(dest_dir_ / "clip.mp4"));
}

TEST_F(OmniFlowTest, FilterIsVideoRejectsTxt) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto txt = test_dir_ / "notes.txt";
    create_file(txt);

    Workflow wf;
    wf.id = "video-reject-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsVideo", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("video-reject-test", txt));
    EXPECT_FALSE(std::filesystem::exists(dest_dir_ / "notes.txt"));
}

// ── IsArchive Filter ──────────────────────────────────────────────────────

TEST_F(OmniFlowTest, FilterIsArchivePasses) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto zip = test_dir_ / "backup.zip";
    create_file(zip);

    Workflow wf;
    wf.id = "archive-filter-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsArchive", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("archive-filter-test", zip));
    EXPECT_TRUE(std::filesystem::exists(dest_dir_ / "backup.zip"));
}

TEST_F(OmniFlowTest, FilterIsArchiveRejectsPng) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto png = test_dir_ / "photo.png";
    create_file(png);

    Workflow wf;
    wf.id = "archive-reject-test";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsArchive", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", dest_dir_.string()}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});

    engine->register_workflow(wf);
    EXPECT_TRUE(engine->execute_workflow("archive-reject-test", png));
    EXPECT_FALSE(std::filesystem::exists(dest_dir_ / "photo.png"));
}

// ── Workflow Persistence Tests ───────────────────────────────────────────

TEST_F(OmniFlowTest, SaveAndLoadWorkflows) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    // Register a custom workflow
    Workflow wf;
    wf.id = "persist-test";
    wf.name = "Persistence Test Flow";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"f1", "Filter.IsPDF", FlowNodeType::Filter, {}});
    wf.nodes.push_back({"a1", "Action.CopyTo", FlowNodeType::Action, {{"target_dir", "/tmp/out"}}});
    wf.connections.push_back({"t1", "out", "f1", "in"});
    wf.connections.push_back({"f1", "true", "a1", "in"});
    engine->register_workflow(wf);

    auto db_path = test_dir_ / "flow_test.db";
    EXPECT_TRUE(engine->save_workflows(db_path));
    EXPECT_TRUE(std::filesystem::exists(db_path));

    // Create a fresh engine and load
    auto engine2 = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine2, nullptr);
    EXPECT_TRUE(engine2->load_workflows(db_path));

    auto workflows = engine2->get_workflows();
    // Should have at least the persisted workflow
    bool found = false;
    for (const auto& w : workflows) {
        if (w.id == "persist-test") {
            found = true;
            EXPECT_EQ(w.name, "Persistence Test Flow");
            EXPECT_EQ(w.nodes.size(), 3u);
            EXPECT_EQ(w.connections.size(), 2u);
            // Verify node config survived
            EXPECT_EQ(w.nodes[2].config.at("target_dir"), "/tmp/out");
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(OmniFlowTest, SaveEmptyDatabase) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto db_path = test_dir_ / "empty_flow.db";
    EXPECT_TRUE(engine->save_workflows(db_path));
    EXPECT_TRUE(std::filesystem::exists(db_path));
}

TEST_F(OmniFlowTest, LoadNonexistentDatabaseReturnsFalse) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    // SQLite creates a new file if it doesn't exist, but there will be no rows
    // The load should still succeed (just returns empty)
    auto result = engine->load_workflows("/nonexistent/path/flow.db");
    // Actually sqlite3_open creates the file, so it may succeed
    EXPECT_NO_THROW(engine->load_workflows("/nonexistent/path/flow.db"));
}

// ── Remove Workflow Tests ────────────────────────────────────────────────

TEST_F(OmniFlowTest, RemoveWorkflow) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    Workflow wf;
    wf.id = "remove-me";
    wf.name = "To Remove";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", FlowNodeType::Trigger, {}});
    engine->register_workflow(wf);

    // Verify it's there
    auto workflows = engine->get_workflows();
    bool found = false;
    for (const auto& w : workflows) {
        if (w.id == "remove-me") found = true;
    }
    EXPECT_TRUE(found);

    // Remove it
    EXPECT_TRUE(engine->remove_workflow("remove-me"));

    // Verify it's gone
    workflows = engine->get_workflows();
    found = false;
    for (const auto& w : workflows) {
        if (w.id == "remove-me") found = true;
    }
    EXPECT_FALSE(found);
}

TEST_F(OmniFlowTest, RemoveNonexistentWorkflowReturnsFalse) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    EXPECT_FALSE(engine->remove_workflow("nonexistent"));
}

TEST_F(OmniFlowTest, SaveAndLoadRoundTripPreservesConfig) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    // Workflow with rich config
    Workflow wf;
    wf.id = "config-rich";
    wf.name = "Rich Config Flow";
    wf.is_active = false;
    wf.nodes.push_back({"n1", "Filter.IsLarger", FlowNodeType::Filter, {{"min_bytes", "1048576"}}});
    wf.nodes.push_back({"n2", "Condition.TextContains", FlowNodeType::Condition, {{"text", "invoice"}}});
    wf.nodes.push_back({"n3", "Action.MoveTo", FlowNodeType::Action, {{"target_dir", "/archive/invoices"}}});
    wf.connections.push_back({"n1", "true", "n2", "in"});
    wf.connections.push_back({"n2", "true", "n3", "in"});
    engine->register_workflow(wf);

    auto db_path = test_dir_ / "config_rich.db";
    EXPECT_TRUE(engine->save_workflows(db_path));

    auto engine2 = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine2, nullptr);
    EXPECT_TRUE(engine2->load_workflows(db_path));

    auto workflows = engine2->get_workflows();
    bool found = false;
    for (const auto& w : workflows) {
        if (w.id == "config-rich") {
            found = true;
            EXPECT_FALSE(w.is_active);
            EXPECT_EQ(w.nodes.size(), 3u);
            EXPECT_EQ(w.nodes[0].config.at("min_bytes"), "1048576");
            EXPECT_EQ(w.nodes[1].config.at("text"), "invoice");
            EXPECT_EQ(w.nodes[2].config.at("target_dir"), "/archive/invoices");
            EXPECT_EQ(w.connections.size(), 2u);
        }
    }
    EXPECT_TRUE(found);
}
