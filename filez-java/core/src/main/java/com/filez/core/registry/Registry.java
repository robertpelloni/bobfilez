package com.filez.core.registry;

import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Supplier;

public final class Registry<T> {
    
    private static final Map<Class<?>, Registry<?>> INSTANCES = new ConcurrentHashMap<>();
    
    private final Map<String, Supplier<T>> factories = new ConcurrentHashMap<>();
    private final Class<T> type;
    
    private Registry(Class<T> type) {
        this.type = type;
    }
    
    @SuppressWarnings("unchecked")
    public static <T> Registry<T> of(Class<T> type) {
        return (Registry<T>) INSTANCES.computeIfAbsent(type, Registry::new);
    }
    
    public void register(String name, Supplier<T> factory) {
        if (factories.containsKey(name)) {
            throw new IllegalArgumentException("Provider already registered: " + name);
        }
        factories.put(name, factory);
    }
    
    public Optional<T> get(String name) {
        Supplier<T> factory = factories.get(name);
        return factory != null ? Optional.of(factory.get()) : Optional.empty();
    }
    
    public T getOrThrow(String name) {
        return get(name).orElseThrow(() -> 
            new IllegalArgumentException("No provider registered with name: " + name + " for type: " + type.getName())
        );
    }
    
    public Set<String> names() {
        return Set.copyOf(factories.keySet());
    }
    
    public boolean has(String name) {
        return factories.containsKey(name);
    }
    
    public void clear() {
        factories.clear();
    }
}
