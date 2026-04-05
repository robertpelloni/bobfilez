#include <bobgui/bobgui.h>

static void
activate (BobguiApplication *app, gpointer user_data)
{
    BobguiWindow *window;
    BobguiBox *box;
    BobguiWidget *title;
    BobguiWidget *body;

    window = bobgui_application_window_new (app);
    bobgui_window_set_title (window, "bobfilez BobGUI Demo");
    bobgui_window_set_default_size (window, 960, 640);

    box = bobgui_box_new (BOBGUI_ORIENTATION_VERTICAL, 16);
    bobgui_widget_set_margin_top (BOBGUI_WIDGET (box), 32);
    bobgui_widget_set_margin_bottom (BOBGUI_WIDGET (box), 32);
    bobgui_widget_set_margin_start (BOBGUI_WIDGET (box), 32);
    bobgui_widget_set_margin_end (BOBGUI_WIDGET (box), 32);

    title = bobgui_label_new ("bobfilez BobGUI Demo");
    body = bobgui_label_new ("This minimal BobGUI frontend is Meson-based and targets bobgui4 as an optional native lane.");

    bobgui_box_append (box, title);
    bobgui_box_append (box, body);
    bobgui_window_set_child (window, BOBGUI_WIDGET (box));
    bobgui_window_present (window);
}

int
main (int argc, char **argv)
{
    BobguiApplication *app;
    int status;

    app = bobgui_application_new ("org.bobfilez.BobguiDemo", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;
}
