#include "fo/c_api/bobfilez_c_api.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
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
    return 0;
}
