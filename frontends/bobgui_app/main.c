#include <bobgui/bobgui.h>

#include <glib.h>

#include <string.h>

typedef struct {
    BobguiWidget *window;
    BobguiWidget *path_entry;
    BobguiWidget *status_label;
    BobguiWidget *output_view;
    gchar *cli_path;
} AppState;

typedef struct {
    AppState *state;
    gchar *operation;
    gchar *target_path;
} CommandRequest;

typedef struct {
    AppState *state;
    gchar *status;
    gchar *output;
} CommandResult;

typedef struct {
    AppState *state;
    const gchar *operation;
} ButtonContext;

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

    bobgui_label_set_text (BOBGUI_LABEL (result->state->status_label), result->status);
    set_output_text (result->state, result->output);

    g_free (result->status);
    g_free (result->output);
    g_free (result);

    return G_SOURCE_REMOVE;
}

static GStrv
build_cli_argv (const gchar *cli_path,
                const gchar *operation,
                const gchar *target_path)
{
    GPtrArray *args = g_ptr_array_new_with_free_func (g_free);

    g_ptr_array_add (args, g_strdup (cli_path));
    g_ptr_array_add (args, g_strdup (operation));
    g_ptr_array_add (args, g_strdup ("--format=json"));

    if (g_strcmp0 (operation, "duplicates") == 0) {
        g_ptr_array_add (args, g_strdup ("--mode=fast"));
    } else if (g_strcmp0 (operation, "hash") == 0) {
        g_ptr_array_add (args, g_strdup ("--threads=4"));
    }

    g_ptr_array_add (args, g_strdup (target_path));
    g_ptr_array_add (args, NULL);

    return (GStrv) g_ptr_array_free (args, FALSE);
}

static gchar *
build_output_text (const gchar *operation,
                   const gchar *stdout_text,
                   const gchar *stderr_text,
                   gint exit_status)
{
    GString *text = g_string_new ("");

    g_string_append_printf (text, "Operation: %s\nExit Status: %d\n\n", operation, exit_status);

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

    return g_string_free (text, FALSE);
}

static gpointer
run_command_thread (gpointer user_data)
{
    CommandRequest *request = user_data;
    CommandResult *result = g_new0 (CommandResult, 1);
    GError *error = NULL;
    gchar *stdout_text = NULL;
    gchar *stderr_text = NULL;
    gint exit_status = 0;
    GStrv argv;

    result->state = request->state;

    argv = build_cli_argv (request->state->cli_path, request->operation, request->target_path);

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
        result->status = g_strdup_printf ("%s failed for %s", request->operation, request->target_path);
        result->output = g_strdup (error != NULL ? error->message : "Unknown subprocess failure.");
        g_clear_error (&error);
    } else {
        result->status = g_strdup_printf ("Completed %s for %s", request->operation, request->target_path);
        result->output = build_output_text (request->operation, stdout_text, stderr_text, exit_status);
    }

    g_strfreev (argv);
    g_free (stdout_text);
    g_free (stderr_text);
    g_free (request->operation);
    g_free (request->target_path);
    g_free (request);

    g_idle_add (apply_command_result, result);
    return NULL;
}

static void
start_operation (AppState *state,
                 const gchar *operation)
{
    const gchar *target_path = bobgui_editable_get_text (BOBGUI_EDITABLE (state->path_entry));
    CommandRequest *request;
    GThread *thread;

    if (state->cli_path == NULL) {
        bobgui_label_set_text (BOBGUI_LABEL (state->status_label), "fo_cli.exe was not found. Set BOBFILEZ_CLI or build the CLI first.");
        set_output_text (state, "Expected one of: build-msvc/cli/fo_cli.exe, build/cli/fo_cli.exe, ../build-msvc/cli/fo_cli.exe, ../build/cli/fo_cli.exe");
        return;
    }

    if (target_path == NULL || target_path[0] == '\0') {
        bobgui_label_set_text (BOBGUI_LABEL (state->status_label), "Enter a filesystem path before running an action.");
        return;
    }

    bobgui_label_set_text (BOBGUI_LABEL (state->status_label), "Running request...");
    set_output_text (state, "Working...\n");

    request = g_new0 (CommandRequest, 1);
    request->state = state;
    request->operation = g_strdup (operation);
    request->target_path = g_strdup (target_path);

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

static ButtonContext *
create_button_context (AppState *state,
                       const gchar *operation)
{
    ButtonContext *context = g_new0 (ButtonContext, 1);
    context->state = state;
    context->operation = operation;
    return context;
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
    BobguiWidget *button_row;
    BobguiWidget *scan_button;
    BobguiWidget *dupes_button;
    BobguiWidget *stats_button;
    BobguiWidget *hash_button;
    BobguiWidget *metadata_button;
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
    subtitle = bobgui_label_new ("This BobGUI lane now drives real bobfilez CLI workflows without killing the app thread: scan, duplicates, statistics, hash, and metadata.");

    path_row = bobgui_box_new (BOBGUI_ORIENTATION_HORIZONTAL, 8);
    path_entry = bobgui_entry_new ();
    bobgui_widget_set_hexpand (path_entry, TRUE);
    bobgui_editable_set_text (BOBGUI_EDITABLE (path_entry), ".");
    bobgui_box_append (BOBGUI_BOX (path_row), bobgui_label_new ("Path:"));
    bobgui_box_append (BOBGUI_BOX (path_row), path_entry);

    button_row = bobgui_box_new (BOBGUI_ORIENTATION_HORIZONTAL, 8);
    scan_button = bobgui_button_new_with_label ("Scan");
    dupes_button = bobgui_button_new_with_label ("Duplicates");
    stats_button = bobgui_button_new_with_label ("Statistics");
    hash_button = bobgui_button_new_with_label ("Hash");
    metadata_button = bobgui_button_new_with_label ("Metadata");

    bobgui_box_append (BOBGUI_BOX (button_row), scan_button);
    bobgui_box_append (BOBGUI_BOX (button_row), dupes_button);
    bobgui_box_append (BOBGUI_BOX (button_row), stats_button);
    bobgui_box_append (BOBGUI_BOX (button_row), hash_button);
    bobgui_box_append (BOBGUI_BOX (button_row), metadata_button);

    status_label = bobgui_label_new ("Ready.");
    scrolled = bobgui_scrolled_window_new ();
    bobgui_widget_set_vexpand (scrolled, TRUE);
    output_view = bobgui_text_view_new ();
    bobgui_scrolled_window_set_child (BOBGUI_SCROLLED_WINDOW (scrolled), output_view);

    bobgui_box_append (BOBGUI_BOX (root_box), title);
    bobgui_box_append (BOBGUI_BOX (root_box), subtitle);
    bobgui_box_append (BOBGUI_BOX (root_box), path_row);
    bobgui_box_append (BOBGUI_BOX (root_box), button_row);
    bobgui_box_append (BOBGUI_BOX (root_box), status_label);
    bobgui_box_append (BOBGUI_BOX (root_box), scrolled);

    bobgui_window_set_child (BOBGUI_WINDOW (window), root_box);

    state->window = window;
    state->path_entry = path_entry;
    state->status_label = status_label;
    state->output_view = output_view;
    state->cli_path = find_cli_path ();

    if (state->cli_path != NULL) {
        gchar *initial = g_strdup_printf ("CLI detected at: %s\n\nEnter a path and click an action button.", state->cli_path);
        set_output_text (state, initial);
        g_free (initial);
    } else {
        set_output_text (state, "fo_cli.exe not found yet. Build the CLI or set BOBFILEZ_CLI to an explicit executable path.");
        bobgui_label_set_text (BOBGUI_LABEL (status_label), "Waiting for fo_cli.exe");
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
