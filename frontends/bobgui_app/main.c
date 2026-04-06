#include <bobgui/bobgui.h>

#include <glib.h>

#ifdef BOBFILEZ_HAVE_C_API
#include <fo/c_api/bobfilez_c_api.h>
#endif

#include <string.h>

typedef struct {
    BobguiWidget *window;
    BobguiWidget *path_entry;
    BobguiWidget *ignore_pattern_entry;
    BobguiWidget *ignore_reason_entry;
    BobguiWidget *status_label;
    BobguiWidget *output_view;
    gchar *cli_path;
    gboolean direct_c_api_available;
} AppState;

typedef struct {
    AppState *state;
    gchar *pattern;
    gchar *reason;
} IgnoreFieldResetContext;

typedef struct {
    AppState *state;
    gchar *operation;
    gchar *target_path;
    gchar *extra_text;
} CommandRequest;

typedef struct {
    AppState *state;
    gchar *operation;
    gchar *status;
    gchar *output;
    gboolean succeeded;
} CommandResult;

typedef struct {
    AppState *state;
    const gchar *operation;
} ButtonContext;

#ifdef BOBFILEZ_HAVE_C_API
typedef char *(*BobfilezDirectApiFn) (const char *root_path);
typedef char *(*BobfilezDirectApiTwoArgFn) (const char *first_value, const char *second_value);
#endif

static gboolean
app_has_direct_c_api (void)
{
#ifdef BOBFILEZ_HAVE_C_API
    return TRUE;
#else
    return FALSE;
#endif
}

static void
free_app_state (AppState *state)
{
    if (state == NULL) {
        return;
    }

    g_free (state->cli_path);
    g_free (state);
}

static gchar *
find_cli_path (void)
{
    const gchar *env_cli = g_getenv ("BOBFILEZ_CLI");
    const gchar *candidates[] = {
        env_cli,
        "build-msvc/cli/fo_cli.exe",
        "build/cli/fo_cli.exe",
        "../build-msvc/cli/fo_cli.exe",
        "../build/cli/fo_cli.exe",
        NULL
    };

    for (guint i = 0; candidates[i] != NULL; ++i) {
        if (candidates[i] == NULL || candidates[i][0] == '\0') {
            continue;
        }

        if (g_file_test (candidates[i], G_FILE_TEST_EXISTS)) {
            return g_strdup (candidates[i]);
        }
    }

    return NULL;
}

static const gchar *
active_backend_name (const AppState *state)
{
    if (state->direct_c_api_available) {
        return "fo_c_api";
    }

    if (state->cli_path != NULL) {
        return "fo_cli";
    }

    return "unavailable";
}

static void
set_output_text (AppState *state,
                 const gchar *text)
{
    BobguiTextBuffer *buffer;

    buffer = bobgui_text_view_get_buffer (BOBGUI_TEXT_VIEW (state->output_view));
    bobgui_text_buffer_set_text (buffer, text != NULL ? text : "", -1);
}

static gboolean
apply_command_result (gpointer user_data)
{
    CommandResult *result = user_data;

    if (result->succeeded) {
        gchar *status_text = make_success_status_text (result->state, result->operation);
        if (g_strcmp0 (result->operation, "ignore-add") == 0
            || g_strcmp0 (result->operation, "ignore-remove") == 0) {
            bobgui_editable_set_text (BOBGUI_EDITABLE (result->state->ignore_pattern_entry), "");
        }
        bobgui_label_set_text (BOBGUI_LABEL (result->state->status_label), status_text);
        g_free (status_text);
    } else {
        bobgui_label_set_text (BOBGUI_LABEL (result->state->status_label), result->status);
    }

    set_output_text (result->state, result->output);

    g_free (result->operation);
    g_free (result->status);
    g_free (result->output);
    g_free (result);

    return G_SOURCE_REMOVE;
}

static const gchar *
default_ignore_pattern (void)
{
    return "thumbs.db";
}

static const gchar *
default_ignore_reason (void)
{
    return "Windows thumbnail cache";
}

static void
reset_ignore_fields (AppState *state,
                     const gchar *pattern,
                     const gchar *reason)
{
    bobgui_editable_set_text (BOBGUI_EDITABLE (state->ignore_pattern_entry),
                              pattern != NULL ? pattern : default_ignore_pattern ());
    bobgui_editable_set_text (BOBGUI_EDITABLE (state->ignore_reason_entry),
                              reason != NULL ? reason : default_ignore_reason ());
}

static gboolean
operation_uses_ignore_pattern (const gchar *operation)
{
    return g_strcmp0 (operation, "ignore-add") == 0
        || g_strcmp0 (operation, "ignore-remove") == 0;
}

static gboolean
operation_requires_path (const gchar *operation)
{
    return g_strcmp0 (operation, "history") != 0
        && g_strcmp0 (operation, "ignore") != 0
        && !operation_uses_ignore_pattern (operation);
}

static gboolean
operation_requires_ignore_reason (const gchar *operation)
{
    return g_strcmp0 (operation, "ignore-add") == 0;
}

static const gchar *
operation_display_name (const gchar *operation)
{
    if (g_strcmp0 (operation, "scan") == 0) {
        return "Scan";
    }
    if (g_strcmp0 (operation, "duplicates") == 0) {
        return "Duplicate Analysis";
    }
    if (g_strcmp0 (operation, "stats") == 0) {
        return "Statistics";
    }
    if (g_strcmp0 (operation, "hash") == 0) {
        return "Hash Inspection";
    }
    if (g_strcmp0 (operation, "metadata") == 0) {
        return "Metadata Summary";
    }
    if (g_strcmp0 (operation, "lint") == 0) {
        return "Lint Summary";
    }
    if (g_strcmp0 (operation, "history") == 0) {
        return "History Listing";
    }
    if (g_strcmp0 (operation, "ignore") == 0) {
        return "Ignore Rule Listing";
    }
    if (g_strcmp0 (operation, "ignore-add") == 0) {
        return "Ignore Rule Add";
    }
    if (g_strcmp0 (operation, "ignore-remove") == 0) {
        return "Ignore Rule Remove";
    }

    return operation;
}

static const gchar *
operation_target_heading (const gchar *operation)
{
    if (operation_requires_path (operation)) {
        return "Path";
    }
    if (operation_uses_ignore_pattern (operation)) {
        return "Ignore Pattern";
    }

    return "Request Scope";
}

static const gchar *
operation_target_label (const gchar *operation,
                        const gchar *target_path)
{
    if (operation_requires_path (operation) || operation_uses_ignore_pattern (operation)) {
        return target_path;
    }

    return "(path-free request)";
}

static void
append_post_action_guidance (GString *text,
                             const gchar *operation)
{
    g_string_append (text, "\n--- next helpful action ---\n");

    if (g_strcmp0 (operation, "ignore-add") == 0) {
        g_string_append (text, "Use 'List Ignore Rules' to confirm the new rule, or 'Reset Ignore Fields' to restore the example values.\n");
        return;
    }

    if (g_strcmp0 (operation, "ignore-remove") == 0) {
        g_string_append (text, "Use 'List Ignore Rules' to confirm the removal, or enter a different pattern to continue editing the rule set.\n");
        return;
    }

    if (g_strcmp0 (operation, "history") == 0) {
        g_string_append (text, "Re-run 'List History' after file operations if you want to inspect the latest recorded activity.\n");
        return;
    }

    if (g_strcmp0 (operation, "ignore") == 0) {
        g_string_append (text, "Use 'Add Ignore Rule' or 'Remove Ignore Rule' to modify the rule set directly from this panel.\n");
        return;
    }

    if (g_strcmp0 (operation, "scan") == 0 || g_strcmp0 (operation, "duplicates") == 0 || g_strcmp0 (operation, "stats") == 0) {
        g_string_append (text, "Adjust the Path field and rerun the action if you want to inspect a different directory.\n");
        return;
    }

    if (g_strcmp0 (operation, "hash") == 0 || g_strcmp0 (operation, "metadata") == 0 || g_strcmp0 (operation, "lint") == 0) {
        g_string_append (text, "Keep the current target if you want to compare another operation against the same path.\n");
        return;
    }

    g_string_append (text, "Use the grouped action rows above to continue exploring this target or switch workflows.\n");
}

static const gchar *
editable_value_or_empty (BobguiWidget *widget)
{
    const gchar *text = bobgui_editable_get_text (BOBGUI_EDITABLE (widget));
    if (text == NULL || text[0] == '\0') {
        return "(empty)";
    }

    return text;
}

static gchar *
make_idle_output_text (const AppState *state)
{
    GString *text = g_string_new ("");

    g_string_append_printf (text, "Panel ready via: %s\n\n", active_backend_name (state));
    g_string_append (text, "Filesystem Actions use the Path field.\n");
    g_string_append (text, "Ignore management uses the Ignore Pattern/Reason fields.\n");
    g_string_append (text, "Operational listings such as history and ignore snapshots are path-free.\n");
    g_string_append (text, "Completed results stay in this panel until you run another action or clear the output.\n\n");
    g_string_append_printf (text, "Current Path: %s\n", editable_value_or_empty (state->path_entry));
    g_string_append_printf (text, "Current Ignore Pattern: %s\n", editable_value_or_empty (state->ignore_pattern_entry));
    g_string_append_printf (text, "Current Ignore Reason: %s\n", editable_value_or_empty (state->ignore_reason_entry));

    return g_string_free (text, FALSE);
}

static gchar *
make_success_status_text (const AppState *state,
                          const gchar *operation)
{
    if (g_strcmp0 (operation, "scan") == 0) {
        return g_strdup_printf ("Scan complete. Path retained: %s", editable_value_or_empty (state->path_entry));
    }
    if (g_strcmp0 (operation, "duplicates") == 0) {
        return g_strdup_printf ("Duplicate analysis complete. Path retained for follow-up actions: %s", editable_value_or_empty (state->path_entry));
    }
    if (g_strcmp0 (operation, "stats") == 0) {
        return g_strdup_printf ("Statistics loaded. Current path is still ready: %s", editable_value_or_empty (state->path_entry));
    }
    if (g_strcmp0 (operation, "hash") == 0) {
        return g_strdup_printf ("Hash inspection complete. Current file target retained: %s", editable_value_or_empty (state->path_entry));
    }
    if (g_strcmp0 (operation, "metadata") == 0) {
        return g_strdup_printf ("Metadata summary loaded. Current path retained for related checks: %s", editable_value_or_empty (state->path_entry));
    }
    if (g_strcmp0 (operation, "lint") == 0) {
        return g_strdup_printf ("Lint summary loaded. Current path retained: %s", editable_value_or_empty (state->path_entry));
    }
    if (g_strcmp0 (operation, "history") == 0) {
        return g_strdup ("History loaded. Re-run after more file operations to inspect newer activity.");
    }
    if (g_strcmp0 (operation, "ignore") == 0) {
        return g_strdup ("Ignore rules loaded. Add or remove a rule if you want to edit the list.");
    }
    if (g_strcmp0 (operation, "ignore-add") == 0) {
        return g_strdup_printf ("Ignore rule added. Reason retained: %s", editable_value_or_empty (state->ignore_reason_entry));
    }
    if (g_strcmp0 (operation, "ignore-remove") == 0) {
        return g_strdup ("Ignore rule removed. Enter another pattern or list rules to confirm the change.");
    }

    return g_strdup ("Request completed.");
}

static gchar *
make_pending_output_text (const gchar *operation,
                          const gchar *target_path,
                          const gchar *extra_text)
{
    GString *text = g_string_new ("Working...\n\n");

    g_string_append_printf (text, "Operation: %s\n", operation_display_name (operation));
    g_string_append (text, "Context\n-------\n");
    g_string_append_printf (text, "%s: %s\n", operation_target_heading (operation), operation_target_label (operation, target_path));

    if (g_strcmp0 (operation, "ignore-add") == 0 && extra_text != NULL && extra_text[0] != '\0') {
        g_string_append_printf (text, "Reason: %s\n", extra_text);
    }

    g_string_append (text, "\nExecution\n---------\n");
    g_string_append (text, "The request is running on a background thread so the UI stays responsive.\n");
    g_string_append (text, "When it completes, the output panel will keep the result so you can compare the next action against it.\n");
    return g_string_free (text, FALSE);
}

static GStrv
build_cli_argv (const gchar *cli_path,
                const gchar *operation,
                const gchar *target_path,
                const gchar *extra_text)
{
    GPtrArray *args = g_ptr_array_new_with_free_func (g_free);

    g_ptr_array_add (args, g_strdup (cli_path));

    if (g_strcmp0 (operation, "ignore") == 0) {
        g_ptr_array_add (args, g_strdup ("ignore"));
        g_ptr_array_add (args, g_strdup ("--format=json"));
    } else if (g_strcmp0 (operation, "ignore-add") == 0) {
        g_ptr_array_add (args, g_strdup ("ignore"));
        g_ptr_array_add (args, g_strdup ("add"));
        g_ptr_array_add (args, g_strdup (target_path));
        if (extra_text != NULL && extra_text[0] != '\0') {
            g_ptr_array_add (args, g_strdup (extra_text));
        }
    } else if (g_strcmp0 (operation, "ignore-remove") == 0) {
        g_ptr_array_add (args, g_strdup ("ignore"));
        g_ptr_array_add (args, g_strdup ("remove"));
        g_ptr_array_add (args, g_strdup (target_path));
    } else {
        g_ptr_array_add (args, g_strdup (operation));
        g_ptr_array_add (args, g_strdup ("--format=json"));

        if (g_strcmp0 (operation, "duplicates") == 0) {
            g_ptr_array_add (args, g_strdup ("--mode=fast"));
        } else if (g_strcmp0 (operation, "hash") == 0) {
            g_ptr_array_add (args, g_strdup ("--threads=4"));
        }
    }

    if (operation_requires_path (operation)) {
        g_ptr_array_add (args, g_strdup (target_path));
    }

    g_ptr_array_add (args, NULL);

    return (GStrv) g_ptr_array_free (args, FALSE);
}

static gchar *
build_cli_output_text (const gchar *operation,
                       const gchar *stdout_text,
                       const gchar *stderr_text,
                       gint exit_status)
{
    GString *text = g_string_new ("");

    g_string_append_printf (text,
                            "Operation: %s\nBackend: fo_cli\nExit Status: %d\n\n",
                            operation_display_name (operation),
                            exit_status);
    g_string_append (text, "Result\n------\n");

    if (stdout_text != NULL && stdout_text[0] != '\0') {
        g_string_append (text, stdout_text);
    }

    if (stderr_text != NULL && stderr_text[0] != '\0') {
        if (text->len > 0 && text->str[text->len - 1] != '\n') {
            g_string_append_c (text, '\n');
        }
        g_string_append (text, "\n--- stderr ---\n");
        g_string_append (text, stderr_text);
    }

    append_post_action_guidance (text, operation);
    return g_string_free (text, FALSE);
}

static gchar *
build_direct_output_text (const gchar *operation,
                          const gchar *summary_text)
{
    GString *text = g_string_new ("");

    g_string_append_printf (text,
                            "Operation: %s\nBackend: fo_c_api\nMode: summary\n\n",
                            operation_display_name (operation));
    g_string_append (text, "Result\n------\n");

    if (summary_text != NULL && summary_text[0] != '\0') {
        g_string_append (text, summary_text);
    }

    append_post_action_guidance (text, operation);
    return g_string_free (text, FALSE);
}

static void
run_cli_request (const AppState *state,
                 const gchar *operation,
                 const gchar *target_path,
                 const gchar *extra_text,
                 const gchar *fallback_reason,
                 gchar **status_out,
                 gchar **output_out,
                 gboolean *succeeded_out)
{
    GError *error = NULL;
    gchar *stdout_text = NULL;
    gchar *stderr_text = NULL;
    gint exit_status = 0;
    GStrv argv = build_cli_argv (state->cli_path, operation, target_path, extra_text);

    if (!g_spawn_sync (NULL,
                       argv,
                       NULL,
                       G_SPAWN_DEFAULT,
                       NULL,
                       NULL,
                       &stdout_text,
                       &stderr_text,
                       &exit_status,
                       &error)) {
        *status_out = g_strdup_printf ("%s failed for %s", operation_display_name (operation), operation_target_label (operation, target_path));
        *output_out = g_strdup (error != NULL ? error->message : "Unknown subprocess failure.");
        if (succeeded_out != NULL) {
            *succeeded_out = FALSE;
        }
        g_clear_error (&error);
    } else {
        *status_out = g_strdup_printf ("Completed %s via fo_cli for %s", operation_display_name (operation), operation_target_label (operation, target_path));
        *output_out = build_cli_output_text (operation, stdout_text, stderr_text, exit_status);
        if (succeeded_out != NULL) {
            *succeeded_out = (exit_status == 0);
        }
        if (fallback_reason != NULL && fallback_reason[0] != '\0') {
            GString *prefixed = g_string_new ("Direct fo_c_api path was unavailable; using fo_cli fallback.\nReason: ");
            g_string_append (prefixed, fallback_reason);
            g_string_append (prefixed, "\n\n");
            g_string_append (prefixed, *output_out != NULL ? *output_out : "");
            g_free (*output_out);
            *output_out = g_string_free (prefixed, FALSE);
        }
    }

    g_strfreev (argv);
    g_free (stdout_text);
    g_free (stderr_text);
}

#ifdef BOBFILEZ_HAVE_C_API
static BobfilezDirectApiFn
find_direct_api_function (const gchar *operation)
{
    if (g_strcmp0 (operation, "scan") == 0) {
        return fo_bobfilez_scan_summary_text;
    }
    if (g_strcmp0 (operation, "duplicates") == 0) {
        return fo_bobfilez_duplicates_summary_text;
    }
    if (g_strcmp0 (operation, "stats") == 0) {
        return fo_bobfilez_stats_summary_text;
    }
    if (g_strcmp0 (operation, "hash") == 0) {
        return fo_bobfilez_hash_summary_text;
    }
    if (g_strcmp0 (operation, "metadata") == 0) {
        return fo_bobfilez_metadata_summary_text;
    }
    if (g_strcmp0 (operation, "lint") == 0) {
        return fo_bobfilez_lint_summary_text;
    }
    if (g_strcmp0 (operation, "history") == 0) {
        return fo_bobfilez_history_summary_text;
    }
    if (g_strcmp0 (operation, "ignore") == 0) {
        return fo_bobfilez_ignore_summary_text;
    }
    if (g_strcmp0 (operation, "ignore-remove") == 0) {
        return fo_bobfilez_ignore_remove_summary_text;
    }

    return NULL;
}

static BobfilezDirectApiTwoArgFn
find_direct_two_arg_api_function (const gchar *operation)
{
    if (g_strcmp0 (operation, "ignore-add") == 0) {
        return fo_bobfilez_ignore_add_summary_text;
    }

    return NULL;
}
#endif

static gpointer
run_command_thread (gpointer user_data)
{
    CommandRequest *request = user_data;
    CommandResult *result = g_new0 (CommandResult, 1);

    result->state = request->state;
    result->operation = g_strdup (request->operation);

    if (request->state->direct_c_api_available) {
#ifdef BOBFILEZ_HAVE_C_API
        BobfilezDirectApiFn direct_api = find_direct_api_function (request->operation);
        BobfilezDirectApiTwoArgFn direct_two_arg_api = find_direct_two_arg_api_function (request->operation);
        char *summary_text = NULL;

        if (direct_api == NULL && direct_two_arg_api == NULL) {
            if (request->state->cli_path != NULL) {
                run_cli_request (request->state,
                                 request->operation,
                                 request->target_path,
                                 request->extra_text,
                                 "Requested operation is not exposed by the direct fo_c_api build.",
                                 &result->status,
                                 &result->output,
                                 &result->succeeded);
            } else {
                result->status = g_strdup_printf ("Unsupported operation %s", operation_display_name (request->operation));
                result->output = g_strdup ("This direct C API build does not expose the requested operation.");
            }
        } else {
            if (direct_two_arg_api != NULL) {
                summary_text = direct_two_arg_api (request->target_path, request->extra_text);
            } else {
                summary_text = direct_api (request->target_path);
            }

            if (summary_text == NULL) {
                if (request->state->cli_path != NULL) {
                    run_cli_request (request->state,
                                     request->operation,
                                     request->target_path,
                                     request->extra_text,
                                     fo_bobfilez_last_error (),
                                     &result->status,
                                     &result->output,
                                     &result->succeeded);
                } else {
                    result->status = g_strdup_printf ("%s failed for %s", operation_display_name (request->operation), operation_target_label (request->operation, request->target_path));
                    result->output = g_strdup (fo_bobfilez_last_error ());
                }
            } else {
                result->status = g_strdup_printf ("Completed %s via fo_c_api for %s", operation_display_name (request->operation), operation_target_label (request->operation, request->target_path));
                result->output = build_direct_output_text (request->operation, summary_text);
                result->succeeded = TRUE;
                fo_bobfilez_free_string (summary_text);
            }
        }
#else
        if (request->state->cli_path != NULL) {
            run_cli_request (request->state,
                             request->operation,
                             request->target_path,
                             request->extra_text,
                             "BOBFILEZ_HAVE_C_API was not enabled for this BobGUI build.",
                             &result->status,
                             &result->output,
                             &result->succeeded);
        } else {
            result->status = g_strdup ("Direct C API requested but not compiled in.");
            result->output = g_strdup ("BOBFILEZ_HAVE_C_API was not enabled for this BobGUI build.");
        }
#endif
    } else if (request->state->cli_path != NULL) {
        run_cli_request (request->state,
                         request->operation,
                         request->target_path,
                         request->extra_text,
                         NULL,
                         &result->status,
                         &result->output,
                         &result->succeeded);
    } else {
        result->status = g_strdup ("No backend available.");
        result->output = g_strdup ("Neither fo_c_api nor fo_cli is available for this BobGUI build.");
    }

    g_free (request->operation);
    g_free (request->target_path);
    g_free (request->extra_text);
    g_free (request);

    g_idle_add (apply_command_result, result);
    return NULL;
}

static void
start_operation (AppState *state,
                 const gchar *operation)
{
    const gchar *path_text = bobgui_editable_get_text (BOBGUI_EDITABLE (state->path_entry));
    const gchar *ignore_pattern = bobgui_editable_get_text (BOBGUI_EDITABLE (state->ignore_pattern_entry));
    const gchar *ignore_reason = bobgui_editable_get_text (BOBGUI_EDITABLE (state->ignore_reason_entry));
    const gchar *target_text = operation_uses_ignore_pattern (operation) ? ignore_pattern : path_text;
    CommandRequest *request;
    GThread *thread;

    if (!state->direct_c_api_available && state->cli_path == NULL) {
        bobgui_label_set_text (BOBGUI_LABEL (state->status_label), "No BobGUI backend is available yet.");
        set_output_text (state, "Expected either a direct fo_c_api-enabled BobGUI build or a reachable fo_cli.exe path.");
        return;
    }

    if ((operation_requires_path (operation) || operation_uses_ignore_pattern (operation))
        && (target_text == NULL || target_text[0] == '\0')) {
        bobgui_label_set_text (BOBGUI_LABEL (state->status_label),
                               operation_uses_ignore_pattern (operation)
                                   ? "Enter an ignore pattern before running this action."
                                   : "Enter a filesystem path before running an action.");
        return;
    }

    if (operation_requires_ignore_reason (operation) && ignore_reason != NULL && ignore_reason[0] == '\0') {
        /* Empty reason is allowed, so no validation failure here. */
    }

    {
        gchar *pending_text = make_pending_output_text (operation,
                                                       (operation_requires_path (operation) || operation_uses_ignore_pattern (operation)) ? target_text : "",
                                                       operation_requires_ignore_reason (operation) ? ignore_reason : "");
        bobgui_label_set_text (BOBGUI_LABEL (state->status_label), "Running request...");
        set_output_text (state, pending_text);
        g_free (pending_text);
    }

    request = g_new0 (CommandRequest, 1);
    request->state = state;
    request->operation = g_strdup (operation);
    request->target_path = g_strdup ((operation_requires_path (operation) || operation_uses_ignore_pattern (operation)) ? target_text : "");
    request->extra_text = g_strdup (operation_requires_ignore_reason (operation) ? ignore_reason : "");

    thread = g_thread_new ("bobfilez-bobgui-worker", run_command_thread, request);
    g_thread_unref (thread);
}

static void
action_button_clicked (BobguiWidget *widget,
                       gpointer user_data)
{
    ButtonContext *context = user_data;
    start_operation (context->state, context->operation);
}

static void
clear_output_button_clicked (BobguiWidget *widget,
                             gpointer user_data)
{
    AppState *state = user_data;
    (void) widget;
    gchar *idle_text = make_idle_output_text (state);
    bobgui_label_set_text (BOBGUI_LABEL (state->status_label), "Output cleared.");
    set_output_text (state, idle_text);
    g_free (idle_text);
}

static void
reset_ignore_button_clicked (BobguiWidget *widget,
                             gpointer user_data)
{
    IgnoreFieldResetContext *context = user_data;
    (void) widget;
    reset_ignore_fields (context->state, context->pattern, context->reason);
    bobgui_label_set_text (BOBGUI_LABEL (context->state->status_label), "Ignore fields reset.");
    set_output_text (context->state,
                     "Ignore fields were reset to their default example values.\n\nUse Ignore Add or Ignore Remove to apply a change, or Ignore Rules to refresh the current rule list.");
}

static ButtonContext *
create_button_context (AppState *state,
                       const gchar *operation)
{
    ButtonContext *context = g_new0 (ButtonContext, 1);
    context->state = state;
    context->operation = operation;
    return context;
}

static IgnoreFieldResetContext *
create_ignore_reset_context (AppState *state,
                             const gchar *pattern,
                             const gchar *reason)
{
    IgnoreFieldResetContext *context = g_new0 (IgnoreFieldResetContext, 1);
    context->state = state;
    context->pattern = g_strdup (pattern);
    context->reason = g_strdup (reason);
    return context;
}

static void
free_ignore_reset_context (IgnoreFieldResetContext *context)
{
    if (context == NULL) {
        return;
    }

    g_free (context->pattern);
    g_free (context->reason);
    g_free (context);
}

static void
activate (BobguiApplication *app,
          gpointer user_data)
{
    AppState *state = g_new0 (AppState, 1);
    BobguiWidget *window;
    BobguiWidget *root_box;
    BobguiWidget *title;
    BobguiWidget *subtitle;
    BobguiWidget *path_row;
    BobguiWidget *path_entry;
    BobguiWidget *ignore_row;
    BobguiWidget *ignore_pattern_entry;
    BobguiWidget *ignore_reason_entry;
    BobguiWidget *filesystem_label;
    BobguiWidget *filesystem_row;
    BobguiWidget *operations_label;
    BobguiWidget *operations_row;
    BobguiWidget *utility_label;
    BobguiWidget *utility_row;
    BobguiWidget *scan_button;
    BobguiWidget *dupes_button;
    BobguiWidget *stats_button;
    BobguiWidget *hash_button;
    BobguiWidget *metadata_button;
    BobguiWidget *lint_button;
    BobguiWidget *history_button;
    BobguiWidget *ignore_button;
    BobguiWidget *ignore_add_button;
    BobguiWidget *ignore_remove_button;
    BobguiWidget *reset_ignore_button;
    BobguiWidget *clear_output_button;
    BobguiWidget *status_label;
    BobguiWidget *scrolled;
    BobguiWidget *output_view;

    window = bobgui_application_window_new (app);
    bobgui_window_set_title (BOBGUI_WINDOW (window), "bobfilez BobGUI Demo");
    bobgui_window_set_default_size (BOBGUI_WINDOW (window), 1100, 760);

    root_box = bobgui_box_new (BOBGUI_ORIENTATION_VERTICAL, 12);
    bobgui_widget_set_margin_top (root_box, 20);
    bobgui_widget_set_margin_bottom (root_box, 20);
    bobgui_widget_set_margin_start (root_box, 20);
    bobgui_widget_set_margin_end (root_box, 20);

    title = bobgui_label_new ("bobfilez BobGUI Demo");
    subtitle = bobgui_label_new ("This BobGUI lane now prefers direct fo_c_api integration when available, falls back to fo_cli when needed, and exposes scan, duplicates, statistics, hash, metadata, lint, history, and ignore-rule workflows—including add/remove actions—without blocking the UI thread.");

    path_row = bobgui_box_new (BOBGUI_ORIENTATION_HORIZONTAL, 8);
    path_entry = bobgui_entry_new ();
    bobgui_widget_set_hexpand (path_entry, TRUE);
    bobgui_editable_set_text (BOBGUI_EDITABLE (path_entry), ".");
    bobgui_box_append (BOBGUI_BOX (path_row), bobgui_label_new ("Path:"));
    bobgui_box_append (BOBGUI_BOX (path_row), path_entry);

    ignore_row = bobgui_box_new (BOBGUI_ORIENTATION_HORIZONTAL, 8);
    ignore_pattern_entry = bobgui_entry_new ();
    bobgui_widget_set_hexpand (ignore_pattern_entry, TRUE);
    bobgui_editable_set_text (BOBGUI_EDITABLE (ignore_pattern_entry), default_ignore_pattern ());
    ignore_reason_entry = bobgui_entry_new ();
    bobgui_widget_set_hexpand (ignore_reason_entry, TRUE);
    bobgui_editable_set_text (BOBGUI_EDITABLE (ignore_reason_entry), default_ignore_reason ());
    bobgui_box_append (BOBGUI_BOX (ignore_row), bobgui_label_new ("Ignore Pattern:"));
    bobgui_box_append (BOBGUI_BOX (ignore_row), ignore_pattern_entry);
    bobgui_box_append (BOBGUI_BOX (ignore_row), bobgui_label_new ("Reason:"));
    bobgui_box_append (BOBGUI_BOX (ignore_row), ignore_reason_entry);

    filesystem_label = bobgui_label_new ("Filesystem Actions");
    filesystem_row = bobgui_box_new (BOBGUI_ORIENTATION_HORIZONTAL, 8);
    operations_label = bobgui_label_new ("Operational Listings and Ignore Actions");
    operations_row = bobgui_box_new (BOBGUI_ORIENTATION_HORIZONTAL, 8);
    utility_label = bobgui_label_new ("Utility Actions");
    utility_row = bobgui_box_new (BOBGUI_ORIENTATION_HORIZONTAL, 8);
    scan_button = bobgui_button_new_with_label ("Scan");
    dupes_button = bobgui_button_new_with_label ("Duplicates");
    stats_button = bobgui_button_new_with_label ("Statistics");
    hash_button = bobgui_button_new_with_label ("Hash");
    metadata_button = bobgui_button_new_with_label ("Metadata");
    lint_button = bobgui_button_new_with_label ("Lint");
    history_button = bobgui_button_new_with_label ("List History");
    ignore_button = bobgui_button_new_with_label ("List Ignore Rules");
    ignore_add_button = bobgui_button_new_with_label ("Add Ignore Rule");
    ignore_remove_button = bobgui_button_new_with_label ("Remove Ignore Rule");
    reset_ignore_button = bobgui_button_new_with_label ("Reset Ignore Fields");
    clear_output_button = bobgui_button_new_with_label ("Clear Output");

    bobgui_box_append (BOBGUI_BOX (filesystem_row), scan_button);
    bobgui_box_append (BOBGUI_BOX (filesystem_row), dupes_button);
    bobgui_box_append (BOBGUI_BOX (filesystem_row), stats_button);
    bobgui_box_append (BOBGUI_BOX (filesystem_row), hash_button);
    bobgui_box_append (BOBGUI_BOX (filesystem_row), metadata_button);
    bobgui_box_append (BOBGUI_BOX (filesystem_row), lint_button);
    bobgui_box_append (BOBGUI_BOX (operations_row), history_button);
    bobgui_box_append (BOBGUI_BOX (operations_row), ignore_button);
    bobgui_box_append (BOBGUI_BOX (operations_row), ignore_add_button);
    bobgui_box_append (BOBGUI_BOX (operations_row), ignore_remove_button);
    bobgui_box_append (BOBGUI_BOX (utility_row), reset_ignore_button);
    bobgui_box_append (BOBGUI_BOX (utility_row), clear_output_button);

    status_label = bobgui_label_new ("Ready.");
    scrolled = bobgui_scrolled_window_new ();
    bobgui_widget_set_vexpand (scrolled, TRUE);
    output_view = bobgui_text_view_new ();
    bobgui_scrolled_window_set_child (BOBGUI_SCROLLED_WINDOW (scrolled), output_view);

    bobgui_box_append (BOBGUI_BOX (root_box), title);
    bobgui_box_append (BOBGUI_BOX (root_box), subtitle);
    bobgui_box_append (BOBGUI_BOX (root_box), path_row);
    bobgui_box_append (BOBGUI_BOX (root_box), ignore_row);
    bobgui_box_append (BOBGUI_BOX (root_box), filesystem_label);
    bobgui_box_append (BOBGUI_BOX (root_box), filesystem_row);
    bobgui_box_append (BOBGUI_BOX (root_box), operations_label);
    bobgui_box_append (BOBGUI_BOX (root_box), operations_row);
    bobgui_box_append (BOBGUI_BOX (root_box), utility_label);
    bobgui_box_append (BOBGUI_BOX (root_box), utility_row);
    bobgui_box_append (BOBGUI_BOX (root_box), status_label);
    bobgui_box_append (BOBGUI_BOX (root_box), scrolled);

    bobgui_window_set_child (BOBGUI_WINDOW (window), root_box);

    state->window = window;
    state->path_entry = path_entry;
    state->ignore_pattern_entry = ignore_pattern_entry;
    state->ignore_reason_entry = ignore_reason_entry;
    state->status_label = status_label;
    state->output_view = output_view;
    state->direct_c_api_available = app_has_direct_c_api ();
    state->cli_path = find_cli_path ();

    if (state->direct_c_api_available) {
        gchar *idle_text = make_idle_output_text (state);
        gchar *initial = g_strdup_printf (
            "Direct bobfilez backend detected: %s\n\n%s\nfo_cli fallback path (if needed): %s",
            active_backend_name (state),
            idle_text,
            state->cli_path != NULL ? state->cli_path : "(not found)");
        set_output_text (state, initial);
        g_free (initial);
        g_free (idle_text);
        bobgui_label_set_text (BOBGUI_LABEL (status_label), "Ready via fo_c_api");
    } else if (state->cli_path != NULL) {
        gchar *idle_text = make_idle_output_text (state);
        gchar *initial = g_strdup_printf ("CLI fallback detected at: %s\n\n%s", state->cli_path, idle_text);
        set_output_text (state, initial);
        g_free (initial);
        g_free (idle_text);
        bobgui_label_set_text (BOBGUI_LABEL (status_label), "Ready via fo_cli");
    } else {
        set_output_text (state, "No BobGUI backend was found yet. Build the app with fo_c_api support or provide fo_cli.exe via BOBFILEZ_CLI / standard repo-relative locations.");
        bobgui_label_set_text (BOBGUI_LABEL (status_label), "Waiting for backend");
    }

    g_signal_connect_data (scan_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "scan"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (dupes_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "duplicates"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (stats_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "stats"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (hash_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "hash"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (metadata_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "metadata"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (lint_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "lint"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (history_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "history"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (ignore_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "ignore"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (ignore_add_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "ignore-add"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (ignore_remove_button,
                           "clicked",
                           G_CALLBACK (action_button_clicked),
                           create_button_context (state, "ignore-remove"),
                           (GClosureNotify) g_free,
                           0);
    g_signal_connect_data (reset_ignore_button,
                           "clicked",
                           G_CALLBACK (reset_ignore_button_clicked),
                           create_ignore_reset_context (state, default_ignore_pattern (), default_ignore_reason ()),
                           (GClosureNotify) free_ignore_reset_context,
                           0);
    g_signal_connect_data (clear_output_button,
                           "clicked",
                           G_CALLBACK (clear_output_button_clicked),
                           state,
                           NULL,
                           0);

    g_object_set_data_full (G_OBJECT (window), "bobfilez-state", state, (GDestroyNotify) free_app_state);
    bobgui_window_present (BOBGUI_WINDOW (window));
}

int
main (int argc,
      char **argv)
{
    BobguiApplication *app;
    int status;

    app = bobgui_application_new ("org.bobfilez.BobguiDemo", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;
}
