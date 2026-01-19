// gcc main.c -O3 -Ofast -march=native -o main   $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1)
// k

#include <JavaScriptCore/JavaScript.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "misc.c"
static void on_js_message(WebKitUserContentManager *manager,
                          WebKitJavascriptResult *result, gpointer user_data) {

  JSCValue *value = webkit_javascript_result_get_js_value(result);

  // JSCContext *ctx = jsc_value_get_context(value);

  /* Handle string */
  if (jsc_value_is_string(value)) {
    char *str = jsc_value_to_string(value);
    g_print("JS says: %s (fuck yeah)\n", str);
    g_free(str);
  }
}

static void onDone(GObject *object, GAsyncResult *result, gpointer user_data) {
  JSCValue *value;
  GError *error = NULL;

  value = webkit_web_view_evaluate_javascript_finish(WEBKIT_WEB_VIEW(object),
                                                     result, &error);
  if (!value) {
    g_warning("Error running javascript: %s", error->message);
    g_error_free(error);
    return;
  }

  if (jsc_value_is_number(value)) {
    double n = jsc_value_to_double(value);
    g_print("JS returned number: %f\n", n);
  }
  // webkit_javascript_result_unref(js_result);
}

static void on_load_changed(WebKitWebView *webview, WebKitLoadEvent event,
                            gpointer user_data) {
  if (event != WEBKIT_LOAD_FINISHED)
    return;

  printf("loaded\n");
  // char *js = "add(2, 4)";
  // webkit_web_view_evaluate_javascript(webview, js, -1, NULL, NULL, NULL, onDone,
  //                                     NULL);
}

int isFull = 0;
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event,
                             gpointer user_data) {
  if (event->keyval == GDK_KEY_F11) {
    GtkWindow *window = GTK_WINDOW(widget);
    isFull = !isFull;
    if (isFull)
      gtk_window_fullscreen(window);
    else
      gtk_window_unfullscreen(window);

    return TRUE;
  }
  return FALSE;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *webview;

  WebKitUserContentManager *content = webkit_user_content_manager_new();

  webkit_user_content_manager_register_script_message_handler(content,
                                                              "external");

  g_signal_connect(content, "script-message-received::external",
                   G_CALLBACK(on_js_message), NULL);

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "WebKit2GTK C Example");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

  webview = webkit_web_view_new_with_user_content_manager(content);
  gtk_container_add(GTK_CONTAINER(window), webview);

  g_signal_connect(webview, "load-changed", G_CALLBACK(on_load_changed), NULL);

  // WebKitSettings *settings =
  //     webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webview));
  //
  // webkit_settings_set_enable_developer_extras(settings, TRUE);

  g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

  // size_t file_length;
  // char *fileContent = readFile("index.html", &file_length);
  //
  // char *fileContent = "<html><head><style>h1{color:
  // red;}</style></head><body>"
  //                     "<h1>JS → C</h1>"
  //                     "<button onclick=\"send()\">Send</button>"
  //                     "<script>"
  //                     "function send() {"
  //                     "  window.webkit.messageHandlers.external.postMessage("
  //                     "    'Hello from JavaScript'"
  //                     "  );"
  //                     "}"
  //                     "</script>"
  //                     "</body></html>";
  // char *url = "https://slapstuk-staging2.web.app/";
  char *url = "http://localhost:8080";
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), url);

  gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.example.js2c.fixed",
                            G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
