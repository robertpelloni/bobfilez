/// @file test_omni_remote.cpp
/// @brief Tests for all 15 Omni subsystem engines + RemoteStorageManager.

#include <gtest/gtest.h>
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"

// Omni interfaces
#include "fo/core/omni_graph_interface.hpp"
#include "fo/core/omni_mount_interface.hpp"
#include "fo/core/omni_oracle_interface.hpp"
#include "fo/core/omniaudio_engine_interface.hpp"
#include "fo/core/omniclerk_interface.hpp"
#include "fo/core/omnicluster_interface.hpp"
#include "fo/core/omnicrypt_interface.hpp"
#include "fo/core/omniflow_engine_interface.hpp"
#include "fo/core/omnigit_interface.hpp"
#include "fo/core/omnipeek_interface.hpp"
#include "fo/core/omnisec_engine_interface.hpp"
#include "fo/core/omnishare_interface.hpp"
#include "fo/core/omniterminal_interface.hpp"
#include "fo/core/omniverse_interface.hpp"
#include "fo/core/omnivision_engine_interface.hpp"
#include "fo/core/remote_storage_interface.hpp"

using namespace fo::core;

class OmniRegistrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

// ── OmniGraph Tests ─────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniGraphCreates) {
    auto engine = Registry<IOmniGraph>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

TEST_F(OmniRegistrationTest, OmniGraphUpsertNode) {
    auto engine = Registry<IOmniGraph>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    GraphNode node;
    node.id = "file1";
    node.label = "test.txt";
    node.group = "file";
    EXPECT_NO_THROW(engine->upsert_node(node));
}

TEST_F(OmniRegistrationTest, OmniGraphAddEdge) {
    auto engine = Registry<IOmniGraph>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    GraphNode n1; n1.id = "file1"; engine->upsert_node(n1);
    GraphNode n2; n2.id = "file2"; engine->upsert_node(n2);
    EXPECT_NO_THROW(engine->add_edge("file1", "file2", "related", 1.0f));
}

TEST_F(OmniRegistrationTest, OmniGraphGetConnectedNodes) {
    auto engine = Registry<IOmniGraph>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    GraphNode n1; n1.id = "file1"; engine->upsert_node(n1);
    GraphNode n2; n2.id = "file2"; engine->upsert_node(n2);
    engine->add_edge("file1", "file2", "related", 1.0f);
    auto neighbors = engine->get_connected_nodes("file1");
    EXPECT_EQ(neighbors.size(), 1u);
    EXPECT_EQ(neighbors[0].id, "file2");
}

TEST_F(OmniRegistrationTest, OmniGraphBfsMultiHop) {
    auto engine = Registry<IOmniGraph>::instance().create("default");
    // Create a chain: A -> B -> C
    GraphNode a; a.id = "A"; engine->upsert_node(a);
    GraphNode b; b.id = "B"; engine->upsert_node(b);
    GraphNode c; c.id = "C"; engine->upsert_node(c);
    engine->add_edge("A", "B", "link", 1.0f);
    engine->add_edge("B", "C", "link", 1.0f);

    // Depth 1 from A: only B
    auto d1 = engine->get_connected_nodes("A", 1);
    EXPECT_EQ(d1.size(), 1u);

    // Depth 2 from A: B and C
    auto d2 = engine->get_connected_nodes("A", 2);
    EXPECT_EQ(d2.size(), 2u);
}

TEST_F(OmniRegistrationTest, OmniGraphFindPath) {
    auto engine = Registry<IOmniGraph>::instance().create("default");
    GraphNode a; a.id = "A"; engine->upsert_node(a);
    GraphNode b; b.id = "B"; engine->upsert_node(b);
    GraphNode c; c.id = "C"; engine->upsert_node(c);
    engine->add_edge("A", "B", "link", 1.0f);
    engine->add_edge("B", "C", "link", 1.0f);

    auto path = engine->find_path("A", "C");
    ASSERT_EQ(path.size(), 2u);
    EXPECT_EQ(path[0].source_id, "A");
    EXPECT_EQ(path[0].target_id, "B");
    EXPECT_EQ(path[1].source_id, "B");
    EXPECT_EQ(path[1].target_id, "C");
}

TEST_F(OmniRegistrationTest, OmniGraphFindPathNotFound) {
    auto engine = Registry<IOmniGraph>::instance().create("default");
    GraphNode a; a.id = "A"; engine->upsert_node(a);
    GraphNode z; z.id = "Z"; engine->upsert_node(z);
    auto path = engine->find_path("A", "Z");
    EXPECT_TRUE(path.empty());
}

TEST_F(OmniRegistrationTest, OmniGraphExportTopology) {
    auto engine = Registry<IOmniGraph>::instance().create("default");
    GraphNode a; a.id = "A"; engine->upsert_node(a);
    GraphNode b; b.id = "B"; engine->upsert_node(b);
    engine->add_edge("A", "B", "link", 1.0f);

    auto [nodes, edges] = engine->export_topology();
    // OmniGraphImpl pre-seeds 7 nodes + 8 edges in its constructor
    EXPECT_GE(nodes.size(), 9u); // 7 seeded + 2 added
    EXPECT_GE(edges.size(), 9u); // 8 seeded + 1 added
}

// ── OmniMount Tests ─────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniMountCreates) {
    auto engine = Registry<IOmniMount>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

TEST_F(OmniRegistrationTest, OmniMountMountDoesNotCrash) {
    auto engine = Registry<IOmniMount>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    // Mount may fail (requires Dokany/FUSE), but should not crash
    EXPECT_NO_THROW(engine->mount("ext:cpp", "Z:"));
}

// ── OmniOracle Tests ────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniOracleCreates) {
    auto engine = Registry<IOmniOracle>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

TEST_F(OmniRegistrationTest, OmniOracleQueryDoesNotCrash) {
    auto engine = Registry<IOmniOracle>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    EXPECT_NO_THROW(engine->ask("find large files"));
}

// ── OmniAudio Tests ─────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniAudioCreates) {
    auto engine = Registry<IOmniAudioEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniClerk Tests ─────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniClerkCreates) {
    auto engine = Registry<IOmniClerkEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniCluster Tests ───────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniClusterCreates) {
    auto engine = Registry<IOmniClusterEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniCrypt Tests ─────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniCryptCreates) {
    auto engine = Registry<IOmniCryptEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniFlow Tests ──────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniFlowCreates) {
    auto engine = Registry<IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniGit Tests ───────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniGitCreates) {
    auto engine = Registry<IOmniGitEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniPeek Tests ──────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniPeekCreates) {
    auto engine = Registry<IOmniPeekEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniSec Tests ───────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniSecCreates) {
    auto engine = Registry<IOmniSecEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniShare Tests ─────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniShareCreates) {
    auto engine = Registry<IOmniShareEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniTerminal Tests ──────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniTerminalCreates) {
    auto engine = Registry<IOmniTerminalEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniVerse Tests ─────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniVerseCreates) {
    auto engine = Registry<IOmniVerseEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── OmniVision Tests ────────────────────────────────────────────────────

TEST_F(OmniRegistrationTest, OmniVisionCreates) {
    auto engine = Registry<IOmniVisionEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── RemoteStorageManager Tests ──────────────────────────────────────────

class RemoteStorageTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(RemoteStorageTest, AddAndRetrieveConnection) {
    RemoteStorageManager mgr;
    RemoteConnection conn;
    conn.id = "test-sftp";
    conn.protocol = "sftp";
    conn.host = "example.com";
    conn.port = 22;
    conn.username = "user";
    mgr.add_connection(conn);

    auto conns = mgr.get_connections();
    ASSERT_EQ(conns.size(), 1u);
    EXPECT_EQ(conns[0].id, "test-sftp");
    EXPECT_EQ(conns[0].protocol, "sftp");
    EXPECT_EQ(conns[0].host, "example.com");
}

TEST_F(RemoteStorageTest, RemoveConnection) {
    RemoteStorageManager mgr;
    RemoteConnection conn;
    conn.id = "to-remove";
    mgr.add_connection(conn);
    EXPECT_EQ(mgr.get_connections().size(), 1u);

    mgr.remove_connection("to-remove");
    EXPECT_TRUE(mgr.get_connections().empty());
}

TEST_F(RemoteStorageTest, RemoveNonexistentDoesNotCrash) {
    RemoteStorageManager mgr;
    EXPECT_NO_THROW(mgr.remove_connection("nonexistent"));
}

TEST_F(RemoteStorageTest, MultipleConnections) {
    RemoteStorageManager mgr;
    for (int i = 0; i < 5; ++i) {
        RemoteConnection conn;
        conn.id = "conn-" + std::to_string(i);
        conn.protocol = "sftp";
        conn.host = "host" + std::to_string(i);
        conn.port = 22;
        mgr.add_connection(conn);
    }
    EXPECT_EQ(mgr.get_connections().size(), 5u);
}

TEST_F(RemoteStorageTest, GetProviderNonexistentReturnsNull) {
    RemoteStorageManager mgr;
    auto provider = mgr.get_provider("nonexistent");
    EXPECT_EQ(provider, nullptr);
}

TEST_F(RemoteStorageTest, RemoteConnectionConstruction) {
    RemoteConnection conn{};
    conn.id = "test";
    conn.port = 22;
    EXPECT_EQ(conn.id, "test");
    EXPECT_EQ(conn.port, 22);
    EXPECT_FALSE(conn.is_active);
}
