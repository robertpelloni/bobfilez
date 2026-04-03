package com.filez.core.interfaces;

import java.nio.file.Path;
import java.util.List;

/**
 * Enhanced file operations (TeraCopy/FastCopy parity).
 */
public interface EnhancedFileOps {

    enum CollisionPolicy { Ask, Skip, Overwrite, Rename }

    record CopyOptions(
        boolean moveMode,
        boolean recursive,
        CollisionPolicy collision,
        boolean verify,
        int bufferSizeMb
    ) {}

    record TransferStats(
        long bytesDone,
        long bytesTotal,
        double speedBps,
        int filesDone,
        int filesTotal
    ) {}

    String enqueue(List<Path> sources, Path dest, CopyOptions opts);
    void pause(String jobId);
    void resume(String jobId);
    void cancel(String jobId);
}
