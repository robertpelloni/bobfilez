package com.filez.core.impl;

import com.filez.core.interfaces.FileScanner;
import com.filez.core.interfaces.Hasher;
import com.filez.core.interfaces.MetadataProvider;
import com.filez.core.interfaces.PerceptualHasher;
import com.filez.core.registry.Registry;

public final class CoreModule {

    private CoreModule() {}

    public static void register() {
        Registry.of(FileScanner.class).register("nio", NioFileScanner::new);
        Registry.of(Hasher.class).register("java", JavaHasher::new);
        Registry.of(MetadataProvider.class).register("java", JavaMetadataProvider::new);
        Registry.of(PerceptualHasher.class).register("dhash", () -> new JavaPerceptualHasher("dhash"));
        Registry.of(PerceptualHasher.class).register("ahash", () -> new JavaPerceptualHasher("ahash"));
    }
}
