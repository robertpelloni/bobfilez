package com.filez.core.impl;

import com.filez.core.interfaces.FileScanner;
import com.filez.core.interfaces.Hasher;
import com.filez.core.registry.Registry;

public final class CoreModule {

    private CoreModule() {}

    public static void register() {
        Registry.of(FileScanner.class).register("nio", NioFileScanner::new);
        Registry.of(Hasher.class).register("java", JavaHasher::new);
    }
}
