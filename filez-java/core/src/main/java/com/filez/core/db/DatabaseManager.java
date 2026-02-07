package com.filez.core.db;

import java.nio.file.Path;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

public class DatabaseManager implements AutoCloseable {

    private final Connection connection;
    private static final int SCHEMA_VERSION = 1;

    static {
        try {
            Class.forName("org.sqlite.JDBC");
        } catch (ClassNotFoundException e) {
            throw new RuntimeException("SQLite JDBC driver not found", e);
        }
    }

    public DatabaseManager(Path dbPath) throws SQLException {
        this("jdbc:sqlite:" + dbPath.toAbsolutePath());
    }

    public DatabaseManager(String jdbcUrl) throws SQLException {
        this.connection = DriverManager.getConnection(jdbcUrl);
        connection.setAutoCommit(true);
        initializeSchema();
    }

    public static DatabaseManager inMemory() throws SQLException {
        return new DatabaseManager("jdbc:sqlite::memory:");
    }

    public Connection getConnection() {
        return connection;
    }

    private void initializeSchema() throws SQLException {
        try (Statement stmt = connection.createStatement()) {
            stmt.execute("PRAGMA foreign_keys = ON");
            stmt.execute("PRAGMA journal_mode = WAL");
            
            stmt.execute("""
                CREATE TABLE IF NOT EXISTS schema_version (
                    version INTEGER PRIMARY KEY
                )
                """);
            
            stmt.execute("""
                CREATE TABLE IF NOT EXISTS files (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    path TEXT UNIQUE NOT NULL,
                    size INTEGER NOT NULL,
                    mtime INTEGER NOT NULL,
                    is_dir INTEGER NOT NULL DEFAULT 0
                )
                """);
            
            stmt.execute("""
                CREATE TABLE IF NOT EXISTS file_hashes (
                    file_id INTEGER PRIMARY KEY,
                    fast64 TEXT,
                    strong TEXT,
                    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
                )
                """);
            
            stmt.execute("""
                CREATE TABLE IF NOT EXISTS duplicate_groups (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    fast64 TEXT NOT NULL,
                    size INTEGER NOT NULL,
                    created_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now'))
                )
                """);
            
            stmt.execute("""
                CREATE TABLE IF NOT EXISTS duplicate_members (
                    group_id INTEGER NOT NULL,
                    file_id INTEGER NOT NULL,
                    PRIMARY KEY (group_id, file_id),
                    FOREIGN KEY (group_id) REFERENCES duplicate_groups(id) ON DELETE CASCADE,
                    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
                )
                """);
            
            stmt.execute("CREATE INDEX IF NOT EXISTS idx_files_path ON files(path)");
            stmt.execute("CREATE INDEX IF NOT EXISTS idx_files_size ON files(size)");
            stmt.execute("CREATE INDEX IF NOT EXISTS idx_file_hashes_fast64 ON file_hashes(fast64)");
            stmt.execute("CREATE INDEX IF NOT EXISTS idx_duplicate_groups_fast64 ON duplicate_groups(fast64)");

            stmt.execute("""
                CREATE TABLE IF NOT EXISTS operations (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    type TEXT NOT NULL,
                    source_path TEXT NOT NULL,
                    dest_path TEXT,
                    timestamp INTEGER NOT NULL,
                    status TEXT NOT NULL,
                    details TEXT
                )
                """);
            stmt.execute("CREATE INDEX IF NOT EXISTS idx_operations_timestamp ON operations(timestamp)");
        }
    }

    public void beginTransaction() throws SQLException {
        connection.setAutoCommit(false);
    }

    public void commit() throws SQLException {
        connection.commit();
        connection.setAutoCommit(true);
    }

    public void rollback() throws SQLException {
        connection.rollback();
        connection.setAutoCommit(true);
    }

    @Override
    public void close() throws SQLException {
        if (connection != null && !connection.isClosed()) {
            connection.close();
        }
    }
}
