package com.filez.core.interfaces;

import com.filez.core.types.DuplicateGroup;
import com.filez.core.types.FileInfo;
import java.util.List;

public interface DuplicateFinder {
    
    List<DuplicateGroup> findDuplicates(List<FileInfo> files);
    
    List<DuplicateGroup> findDuplicates(List<FileInfo> files, DuplicateOptions options);
    
    record DuplicateOptions(
        boolean compareBySize,
        boolean compareByFastHash,
        boolean verifyWithStrongHash,
        long minSize
    ) {
        public static DuplicateOptions defaults() {
            return new DuplicateOptions(true, true, true, 1);
        }
        
        public static DuplicateOptions fastOnly() {
            return new DuplicateOptions(true, true, false, 1);
        }
    }
}
