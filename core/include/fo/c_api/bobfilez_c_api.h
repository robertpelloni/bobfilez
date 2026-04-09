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
char* fo_bobfilez_search_json(const char* root_path, const char* query);
char* fo_bobfilez_history_json(const char* reserved);
char* fo_bobfilez_undo_json(const char* reserved);
char* fo_bobfilez_ignore_json(const char* reserved);
char* fo_bobfilez_ignore_add_json(const char* pattern, const char* reason);
char* fo_bobfilez_ignore_remove_json(const char* pattern);

char* fo_bobfilez_scan_summary_text(const char* root_path);
char* fo_bobfilez_duplicates_summary_text(const char* root_path);
char* fo_bobfilez_stats_summary_text(const char* root_path);
char* fo_bobfilez_hash_summary_text(const char* root_path);
char* fo_bobfilez_metadata_summary_text(const char* root_path);
char* fo_bobfilez_lint_summary_text(const char* root_path);
char* fo_bobfilez_search_summary_text(const char* root_path, const char* query);
char* fo_bobfilez_history_summary_text(const char* reserved);
char* fo_bobfilez_undo_summary_text(const char* reserved);
char* fo_bobfilez_ignore_summary_text(const char* reserved);
char* fo_bobfilez_ignore_add_summary_text(const char* pattern, const char* reason);
char* fo_bobfilez_ignore_remove_summary_text(const char* pattern);

// ── Organize (dry-run preview) ────────────────────────────────────────
char* fo_bobfilez_organize_dry_run_json(const char* root_path, const char* destination_template);
char* fo_bobfilez_organize_dry_run_summary_text(const char* root_path, const char* destination_template);

// ── Count helpers ────────────────────────────────────────────────────
char* fo_bobfilez_count_json(const char* root_path);
char* fo_bobfilez_count_summary_text(const char* root_path);

// ── Export ───────────────────────────────────────────────────────────
char* fo_bobfilez_export_json(const char* root_path);
char* fo_bobfilez_export_csv(const char* root_path);
char* fo_bobfilez_export_html(const char* root_path);

// ── Lint ─────────────────────────────────────────────────────────────
char* fo_bobfilez_lint_json(const char* root_path);
char* fo_bobfilez_lint_summary_text(const char* root_path);

// ── Organize ────────────────────────────────────────────────────────
char* fo_bobfilez_organize_dry_run_json(const char* root_path, const char* rule_template);
char* fo_bobfilez_organize_dry_run_summary_text(const char* root_path, const char* rule_template);

// ── Flow ────────────────────────────────────────────────────────────
char* fo_bobfilez_flow_list_json(void);
int  fo_bobfilez_flow_execute(const char* workflow_id, const char* payload_path);

void fo_bobfilez_free_string(char* value);

#ifdef __cplusplus
}
#endif
