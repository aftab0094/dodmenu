// This is the clock widget. include to get the clock widget in gtk layer shell bar

#include <gtk/gtk.h>

static GtkWidget *clock_label = NULL;
static inline void update_clock() {
    if (clock_label) {
        GDateTime *now = g_date_time_new_now_local();
        gchar *time_str = g_date_time_format(now, "%H:%M:%S");
        gtk_label_set_text(GTK_LABEL(clock_label), time_str);
        g_free(time_str);
        g_date_time_unref(now);
    }
}

static gboolean clock_update_callback(gpointer user_data) {
    update_clock();
    return G_SOURCE_CONTINUE; // Continue calling this function
}

static void clock_init(GtkWidget *parent) {
    clock_label = gtk_label_new(NULL);
    gtk_box_append(GTK_BOX(parent), clock_label);
    update_clock(); // Initial update
    g_timeout_add_seconds(1, clock_update_callback, NULL); // Update every second
}

// the clock widget. just do GtkWidget clock = ClockWidget(); and then clock_init(clock) to add the clock to your bar
static inline GtkWidget *ClockWidget() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    return box;
}
