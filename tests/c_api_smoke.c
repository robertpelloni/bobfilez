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
    return 0;
}
