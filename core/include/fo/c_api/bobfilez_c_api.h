#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* fo_bobfilez_last_error(void);

char* fo_bobfilez_scan_json(const char* root_path);
char* fo_bobfilez_duplicates_json(const char* root_path);
char* fo_bobfilez_stats_json(const char* root_path);
char* fo_bobfilez_hash_json(const char* root_path);
char* fo_bobfilez_metadata_json(const char* root_path);
char* fo_bobfilez_lint_json(const char* root_path);
char* fo_bobfilez_history_json(const char* reserved);
char* fo_bobfilez_ignore_json(const char* reserved);

char* fo_bobfilez_scan_summary_text(const char* root_path);
char* fo_bobfilez_duplicates_summary_text(const char* root_path);
char* fo_bobfilez_stats_summary_text(const char* root_path);
char* fo_bobfilez_hash_summary_text(const char* root_path);
char* fo_bobfilez_metadata_summary_text(const char* root_path);
char* fo_bobfilez_lint_summary_text(const char* root_path);
char* fo_bobfilez_history_summary_text(const char* reserved);
char* fo_bobfilez_ignore_summary_text(const char* reserved);

void fo_bobfilez_free_string(char* value);

#ifdef __cplusplus
}
#endif
