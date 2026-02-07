package com.filez.core.db;

import com.filez.core.types.Operation;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public class OperationRepository {
    private final DatabaseManager db;

    public OperationRepository(DatabaseManager db) {
        this.db = db;
    }

    public long log(String type, String sourcePath, String destPath, String status, String details) throws SQLException {
        String sql = "INSERT INTO operations (type, source_path, dest_path, timestamp, status, details) VALUES (?, ?, ?, ?, ?, ?)";
        Connection conn = db.getConnection();
        
        try (PreparedStatement stmt = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS)) {
            stmt.setString(1, type);
            stmt.setString(2, sourcePath);
            stmt.setString(3, destPath);
            stmt.setLong(4, Instant.now().getEpochSecond());
            stmt.setString(5, status);
            stmt.setString(6, details);
            
            stmt.executeUpdate();
            
            try (ResultSet rs = stmt.getGeneratedKeys()) {
                if (rs.next()) {
                    return rs.getLong(1);
                }
            }
        }
        return -1;
    }

    public List<Operation> getAll() throws SQLException {
        List<Operation> ops = new ArrayList<>();
        String sql = "SELECT * FROM operations ORDER BY timestamp DESC";
        
        try (Statement stmt = db.getConnection().createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            while (rs.next()) {
                ops.add(mapRow(rs));
            }
        }
        return ops;
    }

    public Optional<Operation> getLast() throws SQLException {
        String sql = "SELECT * FROM operations ORDER BY timestamp DESC LIMIT 1";
        
        try (Statement stmt = db.getConnection().createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            if (rs.next()) {
                return Optional.of(mapRow(rs));
            }
        }
        return Optional.empty();
    }

    private Operation mapRow(ResultSet rs) throws SQLException {
        return new Operation(
            rs.getLong("id"),
            rs.getString("type"),
            rs.getString("source_path"),
            rs.getString("dest_path"),
            Instant.ofEpochSecond(rs.getLong("timestamp")),
            rs.getString("status"),
            rs.getString("details")
        );
    }
}
