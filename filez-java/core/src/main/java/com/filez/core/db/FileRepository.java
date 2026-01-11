package com.filez.core.db;

import com.filez.core.types.FileInfo;
import com.filez.core.types.Hashes;

import java.nio.file.Path;
import java.sql.*;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public class FileRepository {

    private final DatabaseManager db;

    public FileRepository(DatabaseManager db) {
        this.db = db;
    }

    public FileInfo save(FileInfo file) throws SQLException {
        String sql = """
            INSERT INTO files (path, size, mtime, is_dir)
            VALUES (?, ?, ?, ?)
            ON CONFLICT(path) DO UPDATE SET
                size = excluded.size,
                mtime = excluded.mtime,
                is_dir = excluded.is_dir
            RETURNING id
            """;

        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setString(1, file.path().toString());
            stmt.setLong(2, file.size());
            stmt.setLong(3, file.modifiedTime().getEpochSecond());
            stmt.setInt(4, file.isDirectory() ? 1 : 0);

            ResultSet rs = stmt.executeQuery();
            if (rs.next()) {
                long id = rs.getLong(1);
                return file.withId(id);
            }
            throw new SQLException("Failed to insert file");
        }
    }

    public void saveBatch(List<FileInfo> files) throws SQLException {
        String sql = """
            INSERT INTO files (path, size, mtime, is_dir)
            VALUES (?, ?, ?, ?)
            ON CONFLICT(path) DO UPDATE SET
                size = excluded.size,
                mtime = excluded.mtime,
                is_dir = excluded.is_dir
            """;

        db.beginTransaction();
        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            for (FileInfo file : files) {
                stmt.setString(1, file.path().toString());
                stmt.setLong(2, file.size());
                stmt.setLong(3, file.modifiedTime().getEpochSecond());
                stmt.setInt(4, file.isDirectory() ? 1 : 0);
                stmt.addBatch();
            }
            stmt.executeBatch();
            db.commit();
        } catch (SQLException e) {
            db.rollback();
            throw e;
        }
    }

    public Optional<FileInfo> findByPath(Path path) throws SQLException {
        String sql = "SELECT id, path, size, mtime, is_dir FROM files WHERE path = ?";

        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setString(1, path.toString());
            ResultSet rs = stmt.executeQuery();
            if (rs.next()) {
                return Optional.of(mapRow(rs));
            }
            return Optional.empty();
        }
    }

    public Optional<FileInfo> findById(long id) throws SQLException {
        String sql = "SELECT id, path, size, mtime, is_dir FROM files WHERE id = ?";

        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setLong(1, id);
            ResultSet rs = stmt.executeQuery();
            if (rs.next()) {
                return Optional.of(mapRow(rs));
            }
            return Optional.empty();
        }
    }

    public List<FileInfo> findBySize(long size) throws SQLException {
        String sql = "SELECT id, path, size, mtime, is_dir FROM files WHERE size = ?";
        List<FileInfo> results = new ArrayList<>();

        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setLong(1, size);
            ResultSet rs = stmt.executeQuery();
            while (rs.next()) {
                results.add(mapRow(rs));
            }
        }
        return results;
    }

    public List<FileInfo> findAll() throws SQLException {
        String sql = "SELECT id, path, size, mtime, is_dir FROM files ORDER BY path";
        List<FileInfo> results = new ArrayList<>();

        try (Statement stmt = db.getConnection().createStatement()) {
            ResultSet rs = stmt.executeQuery(sql);
            while (rs.next()) {
                results.add(mapRow(rs));
            }
        }
        return results;
    }

    public void delete(long id) throws SQLException {
        String sql = "DELETE FROM files WHERE id = ?";
        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setLong(1, id);
            stmt.executeUpdate();
        }
    }

    public void deleteByPath(Path path) throws SQLException {
        String sql = "DELETE FROM files WHERE path = ?";
        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setString(1, path.toString());
            stmt.executeUpdate();
        }
    }

    public void saveHashes(long fileId, Hashes hashes) throws SQLException {
        String sql = """
            INSERT INTO file_hashes (file_id, fast64, strong)
            VALUES (?, ?, ?)
            ON CONFLICT(file_id) DO UPDATE SET
                fast64 = excluded.fast64,
                strong = excluded.strong
            """;

        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setLong(1, fileId);
            stmt.setString(2, hashes.fast64());
            stmt.setString(3, hashes.strong().orElse(null));
            stmt.executeUpdate();
        }
    }

    public Optional<Hashes> findHashes(long fileId) throws SQLException {
        String sql = "SELECT fast64, strong FROM file_hashes WHERE file_id = ?";

        try (PreparedStatement stmt = db.getConnection().prepareStatement(sql)) {
            stmt.setLong(1, fileId);
            ResultSet rs = stmt.executeQuery();
            if (rs.next()) {
                String fast64 = rs.getString("fast64");
                String strong = rs.getString("strong");
                return Optional.of(strong != null ? Hashes.of(fast64, strong) : Hashes.ofFast(fast64));
            }
            return Optional.empty();
        }
    }

    public long count() throws SQLException {
        try (Statement stmt = db.getConnection().createStatement()) {
            ResultSet rs = stmt.executeQuery("SELECT COUNT(*) FROM files");
            return rs.next() ? rs.getLong(1) : 0;
        }
    }

    private FileInfo mapRow(ResultSet rs) throws SQLException {
        return new FileInfo(
            rs.getLong("id"),
            Path.of(rs.getString("path")),
            rs.getLong("size"),
            Instant.ofEpochSecond(rs.getLong("mtime")),
            rs.getInt("is_dir") == 1
        );
    }
}
