#include <gtk/gtk.h>

#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800

static void join_channel(GtkWidget *widget, gpointer data)
{
	g_print("Can't join channels yet!\n");
}

static void activate (GtkApplication *app, gpointer user_data)
{
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *button_box;

	/* Setup the top level window widget */
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "PDX IRC Client");
	gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH,
				    WINDOW_HEIGHT);

	/* Initialize button box with horizontal orientation */
	button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	/* Add button_box widget to the window widget */
	gtk_container_add(GTK_CONTAINER(window), button_box);

	/* Initialize button variable */
	button = gtk_button_new_with_label("Join Channel");
	/* Connect button to function join_channel don't pass any extra data */
	g_signal_connect(button, "clicked", G_CALLBACK(join_channel), NULL);
	/* Allow us to specify what the callback function should take as a
	 * parameter by passing it as data (i.e. window). In this case the
	 * main window widget is passed to gtk_widget_destroy() which destroys
	 * the whole GTK window.
	 */
	g_signal_connect_swapped(button, "clicked",
				 G_CALLBACK(gtk_widget_destroy), window);
	gtk_container_add(GTK_CONTAINER(button_box), button);

	gtk_widget_show_all(window);
}

int main(int   argc, char *argv[])
{
	GtkApplication *app;
	int status;
 
	app = gtk_application_new("com.github.bquigs.pdx_irc_client",
				  G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);	
    
	return 0;
}

