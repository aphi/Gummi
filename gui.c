
#include <stdlib.h>

#include "environment.h"
#include "gui.h"

extern gummi_t* gummi;

GtkWidget   *mainwindow;
gchar       *filename=NULL;

void gui_init(GtkBuilder* builder) {
    GtkWidget    *hpaned;
    gint         width, height;
    
    mainwindow = GTK_WIDGET (gtk_builder_get_object (builder, "mainwindow"));
    gtk_window_get_size (GTK_WINDOW (mainwindow), &width, &height);
    
    hpaned= GTK_WIDGET (gtk_builder_get_object(builder, "hpaned" ));
    gtk_paned_set_position (GTK_PANED (hpaned), (width/2)); 
}

void on_menu_new_activate(GtkWidget *widget, void * user) {
    printf("new\n");
    editor_start_search(gummi->editor, "svn", TRUE, TRUE, FALSE);
}

void on_menu_open_activate(GtkWidget *widget, void * user) {
    printf("open\n");
    filename = get_open_filename();
    if (filename != NULL) iofunctions_load_file(gummi->iofunc, filename); 
}

void on_menu_save_activate(GtkWidget *widget, void * user) {
    printf("save\n");
}

void on_menu_saveas_activate(GtkWidget *widget, void * user) {
    printf("saveas\n");
}

gchar* get_open_filename() {
    GtkWidget   *chooser;
       
    chooser = gtk_file_chooser_dialog_new ("Open File...",
                           GTK_WINDOW (mainwindow),
                           GTK_FILE_CHOOSER_ACTION_OPEN,
                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                           GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                           NULL);
                           
    if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
    }
    
    gtk_widget_destroy (chooser);
    return filename;
}
