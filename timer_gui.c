#include <gtk/gtk.h>
#include <stdlib.h>

// Struktur untuk menyimpan widget yang perlu diakses nanti
typedef struct {
    GtkWidget *label_timer;
    GtkWidget *entry_time;
    GtkWidget *button_start;
    gint total_seconds;
    guint timer_id;
    GtkCssProvider *css_provider;
} TimerWidgets;

// Fungsi ini akan dipanggil setiap detik oleh timer
static gboolean update_timer_display(gpointer user_data) {
    TimerWidgets *widgets = (TimerWidgets *)user_data;

    if (widgets->total_seconds > 0) {
        widgets->total_seconds--;

        int h = widgets->total_seconds / 3600;
        int m = (widgets->total_seconds % 3600) / 60;
        int s = widgets->total_seconds % 60;

        char buffer[10];
        g_snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", h, m, s);
        gtk_label_set_text(GTK_LABEL(widgets->label_timer), buffer);

        // Ubah warna teks menjadi merah terang saat mendekati akhir (10 detik terakhir)
        if (widgets->total_seconds <= 10) {
            // DIGANTI: Menggunakan fungsi baru gtk_css_provider_load_from_string
            gtk_css_provider_load_from_string(widgets->css_provider,
                                            "label#timer-label { font-size: 64px; font-weight: bold; color: red; background-color: black; }");
        }
        return G_SOURCE_CONTINUE; // Lanjutkan timer
    } else {
        gtk_label_set_text(GTK_LABEL(widgets->label_timer), "OUTTA TIME!!!"); // Pesan akhir
        gtk_widget_set_sensitive(widgets->button_start, TRUE); // Aktifkan lagi tombol start
        widgets->timer_id = 0;
        return G_SOURCE_REMOVE; // Hentikan timer
    }
}

// Fungsi ini dipanggil ketika tombol "Mulai" diklik
// Fungsi ini dipanggil ketika tombol "Mulai" diklik
static void start_button_clicked(GtkButton *button, gpointer user_data) {
    TimerWidgets *widgets = (TimerWidgets *)user_data;

    if (widgets->timer_id > 0) {
        g_source_remove(widgets->timer_id);
        widgets->timer_id = 0;
    }

    const char *text = gtk_editable_get_text(GTK_EDITABLE(widgets->entry_time));
    
    // --- PERUBAHAN DI SINI ---
    // 1. Baca input sebagai 'menit'
    int minutes = atoi(text);
    // 2. Ubah menit menjadi total detik
    widgets->total_seconds = minutes * 60;
    // --- AKHIR PERUBAHAN ---

    if (widgets->total_seconds > 0) {
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

        // Atur ulang warna label ke kondisi awal (orange)
        gtk_css_provider_load_from_string(widgets->css_provider,
                                        "label#timer-label { font-size: 48px; font-weight: bold; color: orange; background-color: black; }");
        
        // Panggil update display sekali agar langsung muncul angkanya
        update_timer_display(widgets);

        // Set timer untuk memanggil fungsi update_timer_display setiap 1 detik
        widgets->timer_id = g_timeout_add(1000, update_timer_display, widgets);
    }
}

// Fungsi utama yang membangun jendela aplikasi
// Fungsi utama yang membangun jendela aplikasi
static void activate(GtkApplication *app, gpointer user_data) {
    TimerWidgets *widgets = (TimerWidgets *)user_data;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Bomb Timer");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(box), FALSE);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_window_set_child(GTK_WINDOW(window), box);

    widgets->label_timer = gtk_label_new("00:00:00");
    gtk_widget_set_vexpand(widgets->label_timer, TRUE);
    gtk_widget_set_name(widgets->label_timer, "timer-label"); // ID untuk CSS

    widgets->css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(widgets->css_provider,
                                    "label#timer-label { font-size: 48px; font-weight: bold; color: orange; background-color: black; }");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(widgets->css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    gtk_box_append(GTK_BOX(box), widgets->label_timer);

    widgets->entry_time = gtk_entry_new();
    
    // --- PERUBAHAN DI SINI ---
    // Ubah placeholder text agar pengguna tahu
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->entry_time), "Masukkan total MENIT...");
    // --- AKHIR PERUBAHAN ---
    
    gtk_box_append(GTK_BOX(box), widgets->entry_time);

    widgets->button_start = gtk_button_new_with_label("Mulai Detonasi");
    g_signal_connect(widgets->button_start, "clicked", G_CALLBACK(start_button_clicked), widgets);
    gtk_box_append(GTK_BOX(box), widgets->button_start);
    
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    TimerWidgets *widgets = g_malloc(sizeof(TimerWidgets));
    widgets->timer_id = 0;
    widgets->css_provider = NULL;

    GtkApplication *app = gtk_application_new("com.example.bombtimer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), widgets);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    g_free(widgets);

    return status;
}
