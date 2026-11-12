#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <stdio.h>
#include "clock.h"
#include "sway_api.h"


int main(int argc, char *argv[]) {
    gtk_init();

    // var
    GtkWidget *window = gtk_window_new();
    GtkWidget *label0 = gtk_label_new("Custom SwayFX Bar");
    GtkWidget *label1 = gtk_label_new("Custom SwayFX Bar 1");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *spacer0 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *spacer1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *clock = ClockWidget();

    gtk_layer_init_for_window(GTK_WINDOW(window));
    
    clock_init(clock);


    // Spacer setup
    gtk_widget_set_hexpand(spacer0, TRUE);
    gtk_widget_set_hexpand(spacer1, TRUE);

    gtk_widget_set_valign(clock, GTK_ALIGN_CENTER);


    gtk_box_append(GTK_BOX(box), label0);
    gtk_box_append(GTK_BOX(box), spacer0);
    gtk_box_append(GTK_BOX(box), clock);
    gtk_box_append(GTK_BOX(box),  spacer1);
    gtk_box_append(GTK_BOX(box), label1);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_widget_add_css_class(clock, "clock-label");

    // css and window setup
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);

    gtk_layer_set_exclusive_zone(GTK_WINDOW(window), 22);

    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);

    gtk_window_present(GTK_WINDOW(window));

    while (g_list_model_get_n_items(gtk_window_get_toplevels()) > 0) {
        g_main_context_iteration(NULL, TRUE);
    }

    return 0;
}

