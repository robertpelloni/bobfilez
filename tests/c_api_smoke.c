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

    remove("fo_c_api_smoke.db");
    return 0;
}
