#include <gtk/gtk.h>
#include <string.h>
// ===== CODE INSERT =====
#include "../main.h"

int spellActivated = 1;
int caseActivated = 1;

typedef struct Settings
{
	GtkButton* back;
} Settings;


typedef struct Options
{
	GtkCheckButton* spell;
	GtkCheckButton* caseS;

	GtkButton* settings;
}Options;

typedef struct Display
{
	GtkImage* image;
	GtkTextView* d_txt;

	GtkButton* original;
	GtkButton* preprocessed;
	GtkButton* segmentation;

} Display;

typedef struct Action
{
	GtkButton* recognition;
	GtkButton* save;
	GtkButton* copy;

} Action;

typedef struct Ui
{
	GtkWindow* window;

	GtkButton* choose;
	GtkLabel* name;

	GtkStack* stack;
	GtkWidget* mainPanel;
	GtkWidget* settingsPanel;

	gchar* filename;

	Action* action;
	Display* display;
	Options options;
	Settings settings;
} Ui;

// Choose image and display it
static void choose_file(GtkWidget* button, gpointer user_data)
{
	(void) button;
	GtkWidget *dialog;
	Ui *ui = user_data;

	dialog = gtk_file_chooser_dialog_new("Choose an image",
			ui->window,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			"Open",
			GTK_RESPONSE_ACCEPT,
			"Cancel",
			GTK_RESPONSE_CANCEL,
			NULL);

	GtkFileFilter *filter  = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.jpg");
	gtk_file_filter_add_pattern(filter, "*.jpeg");
	gtk_file_filter_add_pattern(filter, "*.png");
	gtk_file_filter_add_pattern(filter, "*.tif");
	gtk_file_filter_add_pattern(filter, "*.bmp");
	gtk_file_filter_add_pattern(filter, "*.gif");
	gtk_file_filter_add_pattern(filter, "*.tiff");
	gtk_file_filter_add_pattern(filter, "*.PNG");

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	gtk_widget_show_all(dialog);

	gint res = gtk_dialog_run (GTK_DIALOG (dialog));

	if(res == GTK_RESPONSE_ACCEPT)
	{
		ui->filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_label_set_text(ui->name, g_path_get_basename(ui->filename));

		gtk_image_set_from_file(ui->display->image, ui->filename);

		g_print("%s\n", ui->filename);
		gtk_widget_set_sensitive(GTK_WIDGET(ui->action->recognition), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui->action->save), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui->action->copy), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui->display->original), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui->display->preprocessed), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ui->display->segmentation), FALSE);
	}

	gtk_widget_destroy(dialog);
}

static void ocr(GtkWidget* button, gpointer user_data)
{
	Ui* ui = user_data;

	gchar *text = g_locale_to_utf8(DoTheJob(ui->filename,spellActivated,caseActivated),-1,NULL,NULL,NULL);

	gtk_text_buffer_set_text(gtk_text_view_get_buffer(ui->display->d_txt), text, -1);

	gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->action->save), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->action->copy), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->display->original), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->display->preprocessed), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->display->segmentation), TRUE);
}

static void save_text(GtkWidget* button, gpointer user_data)
{
	Ui *ui = user_data;
	GtkWidget* dialog;
	GtkFileChooser *chooser;
	(void) button;

	dialog = gtk_file_chooser_dialog_new("Save Text",
			ui->window,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			"Save",
			GTK_RESPONSE_ACCEPT,
			"Cancel",
			GTK_RESPONSE_CANCEL,
			NULL);

	chooser = GTK_FILE_CHOOSER (dialog);

	gtk_file_chooser_set_current_name(chooser, "Untitled-segmented-text");

	gint res = gtk_dialog_run (GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		GtkTextBuffer* buffer = gtk_text_view_get_buffer(ui->display->d_txt);

		GtkTextIter start;
		GtkTextIter end;
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);

		char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
		char *filename;

		filename = gtk_file_chooser_get_filename (chooser);
		FILE* file = fopen(filename, "w");
		fprintf(file, "%s",text);
		fclose(file);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);

}

static void isSpell(GtkWidget* button, gpointer user_data)
{
	(void) button;
	(void) user_data;
	spellActivated = !spellActivated;
}

static void isCase(GtkWidget* button, gpointer user_data)
{
	(void) button;
	(void) user_data;
	caseActivated = !caseActivated;
}

static void copy_text(GtkWidget* button, gpointer user_data)
{
	Ui* ui = user_data;
	(void) button;

	GtkClipboard* clip = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(ui->display->d_txt);

	GtkTextIter start;
	GtkTextIter end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);

	char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	gtk_clipboard_set_text(clip,text,-1);
	g_free(text);
}

static void display_original(GtkWidget* button, gpointer user_data)
{
	Ui *ui = user_data;

	gtk_image_set_from_file(ui->display->image, ui->filename);
	gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->display->preprocessed), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->display->segmentation), TRUE);
}

static void display_prepro(GtkWidget* button, gpointer user_data)
{
	Ui *ui = user_data;

	gtk_image_set_from_file(ui->display->image, "tmp/final.bmp");
	gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->display->original), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->display->segmentation), TRUE);
}

static void display_segmented(GtkWidget* button, gpointer user_data)
{
	Ui *ui = user_data;

	gtk_image_set_from_file(ui->display->image, "tmp/after_segment_binarized.bmp");
	gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->display->original), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(ui->display->preprocessed), TRUE);
}

static void active_settings(GtkWidget* button, gpointer user_data)
{
	Ui* ui = user_data;
	(void) button;
	gtk_stack_set_visible_child (ui->stack,
			ui->settingsPanel);
}

static void quit_settings(GtkWidget* button, gpointer user_data)
{
	Ui* ui = user_data;
	(void) button;
	gtk_stack_set_visible_child (ui->stack,
			ui->mainPanel);
}
int UIMain()
{
	gchar* welcome_text =
		"\nHere is where the segmented text is displayed.\nIf you need to edit it, you can also write here everything you want.\nThe displayed and edited text can be saved with the button \"Save the text\".\nIt can also be copied in the Clipboard with the button \"Copy to Clipboard\".\nFinally, you can see the different steps of the process of text recognition with the buttons in the Display Section.\nWe thank you for using our Optical Recognition Application.\0";
	// Initializes GTK
	gtk_init(NULL, NULL);

	// Constructs a GtkBuilder instance
	GtkBuilder* builder = gtk_builder_new();

	// Loads the UI description (Exits if an error occurs).
	GError* error = NULL;
	if(gtk_builder_add_from_file(builder, "sources/user_interface.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	// Gets the widgets
	GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(
				builder, "prime.window"));
	GtkImage* image = GTK_IMAGE(gtk_builder_get_object(
				builder, "display.image"));
	GtkTextView* d_txt = GTK_TEXT_VIEW(gtk_builder_get_object(
				builder, "display.txt"));

	GtkStack* stack = GTK_STACK(gtk_builder_get_object(
				builder,"stack"));
	GtkWidget* mainPanel = GTK_WIDGET(gtk_builder_get_object(
				builder,"page1"));
	GtkWidget* settingsPanel = GTK_WIDGET(gtk_builder_get_object(
				builder,"page2"));

	GtkButton* choose = GTK_BUTTON(
			gtk_builder_get_object(builder, "file_choose.button"));
	GtkLabel* name = GTK_LABEL(gtk_builder_get_object(builder, "file_name.label"));

	GtkButton* recognition = GTK_BUTTON(
			gtk_builder_get_object(builder, "recognition.button"));
	GtkButton* save = GTK_BUTTON(gtk_builder_get_object(builder, "save.button"));
	GtkButton* copy = GTK_BUTTON(gtk_builder_get_object(builder, "copy.button"));

	GtkButton* original = GTK_BUTTON(gtk_builder_get_object(
				builder, "original.button"));
	GtkButton* preprocessed = GTK_BUTTON(gtk_builder_get_object(
				builder, "preprocessed.button"));
	GtkButton* segmentation = GTK_BUTTON(gtk_builder_get_object(
				builder, "segmentation.button"));

	GtkCheckButton* spell = GTK_CHECK_BUTTON(gtk_builder_get_object(
				builder, "spell.checkbutton"));
	GtkCheckButton* caseS = GTK_CHECK_BUTTON(gtk_builder_get_object(
				builder, "case.checkbutton"));

	GtkButton* settings = GTK_BUTTON(gtk_builder_get_object(
				builder, "settings.button"));

	GtkButton* back = GTK_BUTTON(gtk_builder_get_object(
				builder, "settings_back.button"));
	// Set Up the UI
	gtk_widget_set_sensitive(GTK_WIDGET(segmentation), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(recognition), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(save), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(copy), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(original), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(preprocessed), FALSE);

	GtkTextBuffer* buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, welcome_text, -1);
	gtk_text_view_set_buffer(d_txt, buffer);

	// Creates the "Ui" structure
	Action action =
	{
		.recognition = recognition,
		.save = save,
		.copy = copy,
	};

	Display display =
	{
		.image = image,
		.d_txt = d_txt,

		.original = original,
		.preprocessed = preprocessed,
		.segmentation = segmentation,
	};

	Ui ui =
	{
		.window = window,

		.choose = choose,
		.name = name,
		.filename = "",

		.stack = stack,
		.mainPanel = mainPanel,
		.settingsPanel = settingsPanel,

		.action = &action,
		.options =
		{
			.spell = spell,
			.caseS = caseS,
			.settings = settings,
		},
		.settings =
		{
			.back = back,
		},

		.display = &display,
	};

	// Connects event handlers
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(choose, "clicked", G_CALLBACK(choose_file), &ui);
	g_signal_connect(recognition, "clicked", G_CALLBACK(ocr), &ui);
	g_signal_connect(save, "clicked", G_CALLBACK(save_text), &ui);

	g_signal_connect(copy, "clicked", G_CALLBACK(copy_text), &ui);
	g_signal_connect(original, "clicked", G_CALLBACK(display_original), &ui);
	g_signal_connect(preprocessed, "clicked", G_CALLBACK(display_prepro), &ui);
	g_signal_connect(segmentation, "clicked", G_CALLBACK(display_segmented), &ui);

	g_signal_connect(spell, "toggled", G_CALLBACK(isSpell), NULL);
	g_signal_connect(caseS, "toggled", G_CALLBACK(isCase), NULL);
	g_signal_connect(settings, "clicked", G_CALLBACK(active_settings), &ui);
	g_signal_connect(back, "clicked",G_CALLBACK(quit_settings), &ui);
	gtk_widget_show(GTK_WIDGET(window));

	// Runs the main loop
	gtk_main();
	// Exits
	return 0;
}
