package com.filez.core.db;

import com.filez.core.types.DuplicateGroup;

import java.nio.file.Path;
import java.sql.*;
import java.util.ArrayList;
import java.util.List;

public class DuplicateRepository {

    private final DatabaseManager db;

    public DuplicateRepository(DatabaseManager db) {
        this.db = db;
    }

    public long saveGroup(DuplicateGroup group) throws SQLException {
        String groupSql = """
            INSERT INTO duplicate_groups (fast64, size)
            VALUES (?, ?)
            """;

        String memberSql = """
            INSERT INTO duplicate_members (group_id, file_id)
            SELECT ?, id FROM files WHERE path = ?
            """;

        db.beginTransaction();
        try {
            long groupId;
            try (PreparedStatement stmt = db.getConnection().prepareStatement(groupSql, Statement.RETURN_GENERATED_KEYS)) {
                stmt.setString(1, group.fast64());
                stmt.setLong(2, group.size());
                stmt.executeUpdate();
                ResultSet rs = stmt.getGeneratedKeys();
                if (!rs.next()) {
                    throw new SQLException("Failed to create duplicate group");
                }
                groupId = rs.getLong(1);
            }

            try (PreparedStatement stmt = db.getConnection().prepareStatement(memberSql)) {
                for (Path filePath : group.files()) {
                    stmt.setLong(1, groupId);
                    stmt.setString(2, filePath.toString());
                    stmt.addBatch();
                }
                stmt.executeBatch();
            }

            db.commit();
            return groupId;
        } catch (SQLException e) {
            db.rollback();
            throw e;
        }
    }

    public List<DuplicateGroup> findAll() throws SQLException {
        String sql = """
            SELECT dg.id, dg.fast64, dg.size, f.path
            FROM duplicate_groups dg
            JOIN duplicate_members dm ON dg.id = dm.group_id
            JOIN files f ON dm.file_id = f.id
            ORDER BY dg.id, f.path
            """;

        List<DuplicateGroup> groups = new ArrayList<>();
        Long currentGroupId = null;
        String currentHash = null;
        long currentSize = 0;
        List<Path> currentFiles = new ArrayList<>();

        try (Statement stmt = db.getConnection().createStatement()) {
            ResultSet rs = stmt.executeQuery(sql);
            while (rs.next()) {
                long groupId = rs.getLong("id");
                if (currentGroupId != null && groupId != currentGroupId) {
                    groups.add(new DuplicateGroup(currentSize, currentHash, List.copyOf(currentFiles)));
                    currentFiles.clear();
                }
                currentGroupId = groupId;
                currentHash = rs.getString("fast64");
                currentSize = rs.getLong("size");
                currentFiles.add(Path.of(rs.getString("path")));
            }
            if (currentGroupId != null && !currentFiles.isEmpty()) {
                groups.add(new DuplicateGroup(currentSize, currentHash, List.copyOf(currentFiles)));
            }
        }
        return groups;
    }

    public List<DuplicateGroup> findByHash(String fast64) throws SQLException {
        String sql = """
            SELECT dg.id, dg.fast64, dg.size, f.path
            FROM duplicate_groups dg
            JOIN duplicate_members dm ON dg.id = dm.group_id
            JOIN files f ON dm.file_id = f.id
            WHERE dg.fast64 = ?
            ORDER BY dg.id, f.path
            """;

        List<DuplicateGroup> groups = new ArrayList<>();
        Long currentGroupId = null;
        String currentHash = null;
        long currentSize = 0;
        List<Path> currentFiles = new ArrayList<>();

        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setString(1, fast64);
            ResultSet rs = stmt.executeQuery();
            while (rs.next()) {
                long groupId = rs.getLong("id");
                if (currentGroupId != null && groupId != currentGroupId) {
                    groups.add(new DuplicateGroup(currentSize, currentHash, List.copyOf(currentFiles)));
                    currentFiles.clear();
                }
                currentGroupId = groupId;
                currentHash = rs.getString("fast64");
                currentSize = rs.getLong("size");
                currentFiles.add(Path.of(rs.getString("path")));
            }
            if (currentGroupId != null && !currentFiles.isEmpty()) {
                groups.add(new DuplicateGroup(currentSize, currentHash, List.copyOf(currentFiles)));
            }
        }
        return groups;
    }

    public void deleteGroup(long groupId) throws SQLException {
        String sql = "DELETE FROM duplicate_groups WHERE id = ?";
        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setLong(1, groupId);
            stmt.executeUpdate();
        }
    }

    public void deleteAll() throws SQLException {
        try (Statement stmt = db.getConnection().createStatement()) {
            stmt.executeUpdate("DELETE FROM duplicate_groups");
        }
    }

    public long count() throws SQLException {
        try (Statement stmt = db.getConnection().createStatement()) {
            ResultSet rs = stmt.executeQuery("SELECT COUNT(*) FROM duplicate_groups");
            return rs.next() ? rs.getLong(1) : 0;
        }
    }
}
