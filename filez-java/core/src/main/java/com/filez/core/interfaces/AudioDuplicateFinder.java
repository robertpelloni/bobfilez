package com.filez.core.interfaces;

import com.filez.core.types.AudioDuplicateGroup;
import com.filez.core.types.FileInfo;
import java.util.List;

public interface AudioDuplicateFinder {
    
    List<AudioDuplicateGroup> findDuplicates(List<FileInfo> files, AudioCompareOptions options);
    
    default List<AudioDuplicateGroup> findDuplicates(List<FileInfo> files) {
        return findDuplicates(files, AudioCompareOptions.defaults());
    }
    
    record AudioCompareOptions(
        boolean useTags,
        boolean useContent,
        boolean usePrecise,
        double tagThreshold,
        double contentThreshold,
        double preciseThreshold,
        int durationToleranceMs,
        int folderGroup1,
        int folderGroup2
    ) {
        public static AudioCompareOptions defaults() {
            return new AudioCompareOptions(
                true, true, true,
                0.80, 0.80, 0.85,
                3000, -1, -1
            );
        }
        
        public static AudioCompareOptions preciseOnly() {
            return new AudioCompareOptions(
                false, false, true,
                0, 0, 0.90,
                3000, -1, -1
            );
        }
        
        public static AudioCompareOptions largeCollection() {
            return new AudioCompareOptions(
                false, false, true,
                0, 0, 0.85,
                3000, -1, -1
            );
        }
    }
}
