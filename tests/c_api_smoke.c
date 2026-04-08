#include "fo/c_api/bobfilez_c_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
#ifdef _WIN32
    _putenv_s("BOBFILEZ_DB_PATH", "fo_c_api_smoke.db");
#else
    setenv("BOBFILEZ_DB_PATH", "fo_c_api_smoke.db", 1);
#endif
    char *json = fo_bobfilez_scan_json(".");
    if (json == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_scan_json failed: %s\n", error != NULL ? error : "(no error)");
        return 1;
    }

    if (strstr(json, "\"path\"") == NULL) {
        fprintf(stderr, "scan JSON did not contain expected path field\n");
        fo_bobfilez_free_string(json);
        return 2;
    }
    fo_bobfilez_free_string(json);

    char *summary = fo_bobfilez_scan_summary_text(".");
    if (summary == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_scan_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 3;
    }

    if (strstr(summary, "Scan Summary") == NULL) {
        fprintf(stderr, "scan summary did not contain expected title\n");
        fo_bobfilez_free_string(summary);
        return 4;
    }

    fo_bobfilez_free_string(summary);

    char *lint = fo_bobfilez_lint_summary_text(".");
    if (lint == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_lint_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 5;
    }

    if (strstr(lint, "Lint Summary") == NULL) {
        fprintf(stderr, "lint summary did not contain expected title\n");
        fo_bobfilez_free_string(lint);
        return 6;
    }

    fo_bobfilez_free_string(lint);

    char *history = fo_bobfilez_history_summary_text("");
    if (history == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_history_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 7;
    }

    if (strstr(history, "History Summary") == NULL) {
        fprintf(stderr, "history summary did not contain expected title\n");
        fo_bobfilez_free_string(history);
        return 8;
    }
    fo_bobfilez_free_string(history);

    char *ignore_add = fo_bobfilez_ignore_add_summary_text("fo_c_api_smoke.tmp", "smoke test ignore rule");
    if (ignore_add == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_ignore_add_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 9;
    }

    if (strstr(ignore_add, "Ignore Rule Added") == NULL) {
        fprintf(stderr, "ignore add summary did not contain expected title\n");
        fo_bobfilez_free_string(ignore_add);
        return 10;
    }
    fo_bobfilez_free_string(ignore_add);

    char *ignore = fo_bobfilez_ignore_summary_text("");
    if (ignore == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_ignore_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 11;
    }

    if (strstr(ignore, "Ignore Rules Summary") == NULL) {
        fprintf(stderr, "ignore summary did not contain expected title\n");
        fo_bobfilez_free_string(ignore);
        return 12;
    }
    fo_bobfilez_free_string(ignore);

    char *ignore_remove = fo_bobfilez_ignore_remove_summary_text("fo_c_api_smoke.tmp");
    if (ignore_remove == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_ignore_remove_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 13;
    }

    if (strstr(ignore_remove, "Ignore Rule Removed") == NULL) {
        fprintf(stderr, "ignore remove summary did not contain expected title\n");
        fo_bobfilez_free_string(ignore_remove);
        return 14;
    }
    fo_bobfilez_free_string(ignore_remove);

    char *search = fo_bobfilez_search_summary_text(".", "test");
    if (search == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_search_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 15;
    }

    if (strstr(search, "Search Results") == NULL) {
        fprintf(stderr, "search summary did not contain expected title\n");
        fo_bobfilez_free_string(search);
        return 16;
    }
    fo_bobfilez_free_string(search);

    char *undo = fo_bobfilez_undo_summary_text("");
    if (undo == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_undo_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 17;
    }

    // It might say "Nothing to undo" or "Undo Successful"
    if (strstr(undo, "undo") == NULL && strstr(undo, "Undo") == NULL) {
        fprintf(stderr, "undo summary did not contain expected text\n");
        fo_bobfilez_free_string(undo);
        return 18;
    }
    fo_bobfilez_free_string(undo);

    remove("fo_c_api_smoke.db");

    /* Smoke-test organize dry-run */
    char *organize = fo_bobfilez_organize_dry_run_summary_text(".", "sorted/{name}.{ext}");
    if (organize == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_organize_dry_run_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 19;
    }

    if (strstr(organize, "Organize Preview") == NULL) {
        fprintf(stderr, "organize summary did not contain expected title\n");
        fo_bobfilez_free_string(organize);
        return 20;
    }
    fo_bobfilez_free_string(organize);

    /* Smoke-test count */
    char *count = fo_bobfilez_count_summary_text(".");
    if (count == NULL) {
        const char *error = fo_bobfilez_last_error();
        fprintf(stderr, "fo_bobfilez_count_summary_text failed: %s\n", error != NULL ? error : "(no error)");
        return 21;
    }

    if (strstr(count, "Count Summary") == NULL) {
        fprintf(stderr, "count summary did not contain expected title\n");
        fo_bobfilez_free_string(count);
        return 22;
    }
    fo_bobfilez_free_string(count);

    return 0;
}
