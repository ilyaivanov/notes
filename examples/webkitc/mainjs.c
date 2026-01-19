#include <JavaScriptCore/JavaScript.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

/* Called when JS sends a message */
static void on_js_message(WebKitUserContentManager *manager,
                          WebKitJavascriptResult *result, gpointer user_data) {
  JSGlobalContextRef ctx = webkit_javascript_result_get_global_context(result);
  JSValueRef value = webkit_javascript_result_get_value(result);

  /* Handle string */
  if (JSValueIsString(ctx, value)) {
    JSStringRef js_str = JSValueToStringCopy(ctx, value, NULL);
    size_t size = JSStringGetMaximumUTF8CStringSize(js_str);
    char *cstr = g_malloc(size);

    JSStringGetUTF8CString(js_str, cstr, size);
    g_print("JS says: %s\n", cstr);

    g_free(cstr);
    JSStringRelease(js_str);
  }
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
  gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
  gtk_window_set_title(GTK_WINDOW(window), "JS → C (Fixed)");

  webview = webkit_web_view_new_with_user_content_manager(content);
  gtk_container_add(GTK_CONTAINER(window), webview);

  WebKitSettings *settings =
      webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webview));

  webkit_settings_set_enable_developer_extras(settings, TRUE);

  char *fileContent = "<html><head><style>h1{color: red;}</style></head><body>"
                      "<h1>JS → C</h1>"
                      "<button onclick=\"send()\">Send</button>"
                      "<script>"
                      "function send() {"
                      "  window.webkit.messageHandlers.external.postMessage("
                      "    'Hello from JavaScript'"
                      "  );"
                      "}"
                      "</script>"
                      "</body></html>";
  webkit_web_view_load_html(WEBKIT_WEB_VIEW(webview), fileContent, NULL);

  gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.example.js2c.fixed", G_APPLICATION_FLAGS_NONE);

  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
