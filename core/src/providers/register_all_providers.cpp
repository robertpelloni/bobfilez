#include "fo/core/provider_registration.hpp"
#include <iostream>

namespace fo::core {
    // Forward declarations for individual provider registration functions.
    void register_hasher_dhash();
    void register_linter_std();
    void register_file_watcher_native();
    void register_audit_logger();
    void register_treemap_engine();
    void register_data_pruner();
    void register_self_healing_engine();
    void register_hierarchy_engine();
    void register_nexus();
    void register_clip_search_engine();
    void register_document_embedder();
    void register_virtual_fs();
    void register_time_machine();
    void register_semantic_tagging();
    void register_swarm_engine();
    void register_neural_bridge();
    void register_perceptual_opencv();
    void register_omni_graph();
    void register_omni_mount();
    void register_omni_oracle();
    void register_omniaudio_engine();
    void register_omniclerk_engine();
    void register_omnicluster_engine();
    void register_omnicrypt_engine();
    void register_omniflow_engine();
    void register_omnigit_engine();
    void register_omnipeek_engine();
    void register_omnisec_engine();
    void register_omnishare_engine();
    void register_omniterminal_engine();
    void register_omniverse_engine();
    void register_omnivision_engine();
    void register_scanner_s3();
    void register_scanner_gdrive();
    void register_scanner_azure();

    void register_extended_providers() {
        register_hasher_dhash();
        register_scanner_s3();
        register_scanner_gdrive();
        register_scanner_azure();
        // Add calls to other provider registration functions here.
    }

    void register_all_providers() {
        register_scanner_std();
#ifdef _WIN32
        register_scanner_win32();
#endif
        register_scanner_dirent();
        register_hasher_fast64();
        register_hasher_sha256();
        register_hasher_xxhash();
        register_hasher_blake3();
        register_metadata_tinyexif();
        register_linter_std();
        register_file_watcher_native();
        register_audit_logger();
        register_treemap_engine();
        register_data_pruner();
        register_self_healing_engine();
        register_hierarchy_engine();
        register_nexus();
        register_clip_search_engine();
        register_document_embedder();
        register_virtual_fs();
        register_time_machine();
        register_semantic_tagging();
        register_swarm_engine();
        register_neural_bridge();
        register_perceptual_opencv();

        // Omni subsystem registrations
        register_omni_graph();
        register_omni_mount();
        register_omni_oracle();
        register_omniaudio_engine();
        register_omniclerk_engine();
        register_omnicluster_engine();
        register_omnicrypt_engine();
        register_omniflow_engine();
        register_omnigit_engine();
        register_omnipeek_engine();
        register_omnisec_engine();
        register_omnishare_engine();
        register_omniterminal_engine();
        register_omniverse_engine();
        register_omnivision_engine();
        
        register_extended_providers();
    }
}
