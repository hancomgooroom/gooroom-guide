#include "config.h"
#include "stdafx.h"

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gdk/gdk.h>
#include <webkit2/webkit2.h>
#include <glib/gi18n-lib.h>

#define fopen_s(fp, fmt, mode)	*(fp)=fopen((fmt), (mode))

// for label
typedef struct _FontStyle {
    char* family;
    GdkColor color;
    int size;
    bool hasUnderline = TRUE;
    bool isBold = FALSE;


    void setColor(guint16 red, guint16 green, guint16 blue)
    {
        this->color.red = red;
        this->color.green = green;
        this->color.blue = blue;
    }

    void setSize(int size)
    {
        this->size = size;
    }

    void setFamily(char* family)
    {
        this->family = family;
    }

    void disableUnderline()
    {
        this->hasUnderline = FALSE;
    }

    void setBold(bool isBold)
    {
        this->isBold = isBold;
    }
}FontStyle;


static GtkWidget* window = NULL;
//static GtkBuilder* builder;
static GtkWidget* btnUndo, *btnRedo;
static GtkWidget* checkBox = NULL;
static GtkWidget* alignment = NULL;
static GtkWidget* headerBar = NULL;

// gtk view
static GtkWidget* beginPage = NULL;
static GtkWidget* endPage = NULL;

// web view
static GtkWidget* webViewPage;
static WebKitWebView* webView = NULL;
static std::vector<std::string> filePaths = std::vector<std::string>();
static std::string curUri;

static gboolean isShowAtBegin = TRUE;
static gboolean isGtkView = TRUE;

//cursor
static GdkCursor* handCursor = NULL;
static GdkCursor* defaultCursor = NULL;

// Functions
bool initUriList();
void createBeginPage();
void createEndPage();
void createWebView();
void createGtkView();

static std::string getExpanduser();
static void setLabelAttributeEx(GtkLabel* label, FontStyle* style);

void loadUri(int index);
void changePageView(GtkWidget* hide, GtkWidget* show);

//Callback
//void closeWindow(GtkWidget* widget, gpointer* data);
void clickedUndo(GtkButton* button, gpointer data);
void clickedRedo(GtkButton* button, gpointer data);
void checkShowAtBegin(GtkButton* button, gpointer data);
void changedLoadWebPage(WebKitWebView* webview, WebKitLoadEvent loadEvent, gpointer data);
void activate(GtkApplication* app);

std::string getHomeDir()
{
    std::string dir = g_get_home_dir();
    if (0 == dir.compare("/root"))
    {
        dir.clear();

        char pwd[PATH_MAX];
        if (NULL == getcwd(pwd, sizeof(pwd)))
            return "";

        char* tok;
        tok = strtok(pwd, "/");
        dir.append("/");
        dir.append(tok);
        dir.append("/");

        tok = strtok(NULL, "/");
        dir.append(tok);
    }

    return dir;
}

void removeBeforeExpand()
{
    std::string expand = getHomeDir();
    expand.append("/.gooroom");

    std::string norun = expand;
    norun.append("/norun");

    if (g_file_test(norun.c_str(), G_FILE_TEST_EXISTS))
    {
        std::ofstream file;
        remove(norun.c_str());
    }

    if (g_file_test(expand.c_str(), G_FILE_TEST_IS_DIR))
        rmdir(expand.c_str());
}

std::string getExpanduser()
{
    std::string expand = getHomeDir();

    expand.append("/.config");
    if (!g_file_test(expand.c_str(), G_FILE_TEST_IS_DIR))
        mkdir(expand.c_str(), 0777);

    expand.append("/gooroom");
    if (!g_file_test(expand.c_str(), G_FILE_TEST_IS_DIR))
        mkdir(expand.c_str(), 0777);

    expand.append("/gooroom-guide");
    if (!g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
        mkdir(expand.c_str(), 0777);

    return expand;
}

bool initUriList()
{
    std::string tocPath;
    tocPath.append(PACKAGE_GUIDEDIR);
    tocPath.append("guide/toc.json");

    Json::Value value;
    std::ifstream jsonDoc(tocPath.c_str(), std::ifstream::binary);
    jsonDoc >> value;
    Json::Value toc = value["paths"];

    std::string path;
    path.append("file://");
    path.append(PACKAGE_GUIDEDIR);
    path.append("guide");

    for (int i = 0; i < toc.size(); i++)
    {
        std::string str;
        str.append(path.c_str());
        str.append(toc[i].asString().c_str());

        filePaths.push_back(str.c_str());
    }

    return TRUE;
}

void loadUri(int index)
{
    webkit_web_view_load_uri(webView, filePaths[index].c_str());
}

void changePageView(GtkWidget* hide, GtkWidget* show)
{
    g_object_ref(hide);
    gtk_container_remove(GTK_CONTAINER(alignment), hide);
    gtk_container_add(GTK_CONTAINER(alignment), show);

    gtk_widget_show_all(show);
}

void clickedUndo(GtkButton* button, gpointer userData)
{
    printf("= = => undo func\n");
    if (webkit_web_view_can_go_back(webView))
       webkit_web_view_go_back(webView);

    if (0 == curUri.compare(filePaths[13]))
        changePageView(endPage, webViewPage);

    if (0 == curUri.compare(filePaths[0]))
        changePageView(beginPage, webViewPage);
}

void clickedRedo(GtkButton* button, gpointer userData)
{
    printf("= = => redo func\n");
    if (webkit_web_view_can_go_forward(webView))
        webkit_web_view_go_forward(webView);

    if (0 == curUri.compare(filePaths[0]))
        changePageView(beginPage, webViewPage);
}

void checkShowAtBegin(GtkButton* button, gpointer userData)
{
    isShowAtBegin = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    // FIXME: memory destroy 될 때 저장도되록 수정해야함
    std::string expand = getExpanduser();

    if (!g_file_test(expand.c_str(), G_FILE_TEST_IS_DIR))
    {
        mkdir(expand.c_str(), 0777);
    }

    expand.append("/norun");
    if (!isShowAtBegin)
    {
        if (!g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
        {
            std::ofstream file;
            file.open(expand.c_str());
            file.close();
        }
    }
    else
    {
        if (g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
        {
            std::ofstream file;
            remove(expand.c_str());
        }
    }

}

//void closeWindow(GtkWidget* widget, gpointer* data)
//{
//}

void changedLoadWebPage(WebKitWebView* webView, WebKitLoadEvent loadEvent, gpointer userData)
{
    switch (loadEvent)
    {
        //case WEBKIT_layout
        //break;
        //case WEBKIT_LOAD_REDIRECTED:
        //break;
        //case WEBKIT_LOAD_COMMITTED:
        //break;
        case WEBKIT_LOAD_FINISHED:
        {
            gtk_widget_set_sensitive(btnUndo, webkit_web_view_can_go_back(webView));
            gtk_widget_set_sensitive(btnRedo, webkit_web_view_can_go_forward(webView));

            const gchar* uri = webkit_web_view_get_uri(webView);

            if (0 == strcmp(uri, filePaths[0].c_str()))
            {
                printf("= = = = => beginPage\n");
                changePageView(webViewPage, beginPage);
            }

            if (0 == strcmp(uri, filePaths[13].c_str()))
            {
                printf("= = = = => endPage\n");
                gtk_widget_set_sensitive(btnRedo, FALSE);
                changePageView(webViewPage, endPage);
            }

            curUri.clear();
            curUri.append(uri);
        }
        break;
    }
}

void createWebView()
{
    initUriList();

    webViewPage = gtk_frame_new(NULL);
    webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

    gtk_container_add(GTK_CONTAINER(webViewPage), GTK_WIDGET(webView));

    g_signal_connect(webView, "load-changed", G_CALLBACK(changedLoadWebPage), NULL);

    loadUri(0);
    gtk_widget_grab_focus(GTK_WIDGET(webView));

    gtk_container_add(GTK_CONTAINER(alignment), webViewPage);
}

void setLabelAttributeEx(GtkLabel* label, FontStyle* style)
{
    PangoAttrList* attrList = pango_attr_list_new();
    PangoAttribute* atFont;
    PangoAttribute* atColor;
    PangoFontDescription* df;

    df = pango_font_description_new();
    if (style->family != NULL)
    {
        pango_font_description_set_family(df, style->family);
    }

    if (style->size != 0)
    {
        pango_font_description_set_size(df, style->size * PANGO_SCALE);
    }

    if (!style->hasUnderline)
    {
        PangoUnderline pul = PANGO_UNDERLINE_NONE;

        pango_attr_list_insert(attrList, pango_attr_underline_new(pul));
    }

    if (style->isBold)
    {
        PangoWeight pw = PANGO_WEIGHT_BOLD;

        pango_attr_list_insert(attrList, pango_attr_weight_new(pw));
    }

    atColor = pango_attr_foreground_new(style->color.red, style->color.green, style->color.blue);
    atFont = pango_attr_font_desc_new(df);

    pango_attr_list_insert(attrList, atFont);
    pango_attr_list_insert(attrList, atColor);

    gtk_label_set_attributes(label, attrList);
    pango_attr_list_unref(attrList);
}

void clickedStart(GtkButton* button, gpointer data)
{
    gtk_widget_set_sensitive(btnUndo, TRUE);
    loadUri(1);
    changePageView(beginPage, webViewPage);
}

static gboolean onChangeCursorInButton(GtkWidget* button, GdkEventCrossing* event)
{
    if (event->type == GDK_ENTER_NOTIFY)
        gdk_window_set_cursor(gtk_widget_get_window(button), handCursor);
    else if (event->type == GDK_LEAVE_NOTIFY)
        gdk_window_set_cursor(gtk_widget_get_window(button), defaultCursor);
}

void createBeginPage()
{
    beginPage = gtk_layout_new(NULL, NULL);

    //gtk_container_add(GTK_CONTAINER(alignment), beginPage);

    GtkCssProvider* cssp = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(cssp, "/kr/gooroom/css/gtkview.css");

    gtk_widget_set_halign(beginPage, GTK_ALIGN_FILL);
    gtk_widget_set_valign(beginPage, GTK_ALIGN_FILL);

    // 배경화면
    GtkWidget* image = gtk_image_new_from_resource("/kr/gooroom/images/bg_start.jpg");
    gtk_layout_put(GTK_LAYOUT(beginPage), image, 0, 0);

    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    gint width, height;
    gtk_window_get_size(GTK_WINDOW(window), &width, &height);

    pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);

    g_object_unref(pixbuf);


    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(beginPage), box);
    gtk_widget_set_size_request(box, width, height);

    GtkWidget* childBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box), childBox, FALSE, FALSE, 50);


    GtkWidget* logo = gtk_image_new_from_resource("/kr/gooroom/images/logo_start.png");
    gtk_box_pack_start(GTK_BOX(childBox), logo, FALSE, FALSE, 30);
    gtk_widget_set_halign(logo, GTK_ALIGN_CENTER);

    GtkWidget* text1 = gtk_label_new(_("Thanks you for choosing Gooroom"));
    gtk_widget_set_name(text1, "beginpage_text1");
    gtk_style_context_add_provider(gtk_widget_get_style_context(text1), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_box_pack_start(GTK_BOX(childBox), text1, TRUE, FALSE, 8);

    GtkWidget* text2 = gtk_label_new(_("The cloud platform promises you a pleasant PC environment with lightweight and fast performance.\n Use the buttons below to see the basic usage of the cloud platform."));
    gtk_widget_set_name(text2, "beginpage_text2");
    gtk_style_context_add_provider(gtk_widget_get_style_context(text2), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_box_pack_start(GTK_BOX(childBox), text2, TRUE, FALSE, 8);

    // 시작하기 버튼
    GtkWidget* startButton = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(childBox), startButton, FALSE, FALSE, 14);
    gtk_widget_set_halign(startButton, GTK_ALIGN_CENTER);

    gtk_widget_set_name(startButton, "startbutton");
    gtk_style_context_add_provider(gtk_widget_get_style_context(startButton), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkWidget* subLabel = gtk_label_new(_("Getting Start"));
    gtk_widget_set_name(subLabel, "gettingstart");
    gtk_container_add(GTK_CONTAINER(startButton), subLabel);
    gtk_style_context_add_provider(gtk_widget_get_style_context(subLabel), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);

    GdkDisplay* d = gtk_widget_get_display(window);
    handCursor = gdk_cursor_new_from_name(d, "pointer");
    defaultCursor = gdk_cursor_new_from_name(d, "default");

    g_signal_connect(startButton, "clicked", G_CALLBACK(clickedStart), NULL);
    g_signal_connect(startButton, "enter-notify-event", G_CALLBACK(onChangeCursorInButton), NULL);
    g_signal_connect(startButton, "leave-notify-event", G_CALLBACK(onChangeCursorInButton), NULL);

    // 부팅 시 마다 확인
    checkBox = gtk_check_button_new();
    GtkWidget* checkLabel = gtk_label_new(_("Show program at Startup"));
    gtk_widget_set_name(checkLabel, "checklabel");
    gtk_style_context_add_provider(gtk_widget_get_style_context(checkLabel), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_container_add(GTK_CONTAINER(checkBox), checkLabel);

    gtk_box_pack_end(GTK_BOX(box), checkBox, FALSE, TRUE, 0);
    gtk_widget_set_halign(checkBox, GTK_ALIGN_END);
    gtk_widget_set_valign(checkBox, GTK_ALIGN_END);

    g_signal_connect(checkBox, "toggled", G_CALLBACK(checkShowAtBegin), NULL);

    std::string expand = getExpanduser();
    expand.append("/norun");
    if (!g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
        isShowAtBegin = TRUE;
    else
        isShowAtBegin = FALSE;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkBox), isShowAtBegin);
}

void createEndPage()
{
    endPage = gtk_layout_new(NULL, NULL);

    GtkCssProvider* cssp = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(cssp, "/kr/gooroom/css/gtkview.css");

    GtkWidget* image = gtk_image_new_from_resource("/kr/gooroom/images/bg_start.jpg");
    gtk_layout_put(GTK_LAYOUT(endPage), image, 0, 0);

    // 배경화면 
    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    gint width, height;
    gtk_window_get_size(GTK_WINDOW(window), &width, &height);

    pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
    g_object_unref(pixbuf);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(endPage), box);
    gtk_widget_set_size_request(box, width, height);

    GtkWidget* childBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box), childBox, FALSE, FALSE, 90);


    GtkWidget* text1 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(text1), _("You can begin the Gooroom platform"));
    gtk_widget_set_name(text1, "endpage_text1");
    gtk_style_context_add_provider(gtk_widget_get_style_context(text1), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_box_pack_start(GTK_BOX(childBox), text1, TRUE, FALSE, 40);
    gtk_widget_set_halign(text1, GTK_ALIGN_CENTER);

    GtkWidget* text2 = gtk_label_new("test");
    gtk_label_set_markup(GTK_LABEL(text2), _("Website : <a href=\"https://www.gooroom.kr/\">https://www.gooroom.kr/</a>"));
    gtk_widget_set_name(text2, "endpage_text2");
//    gtk_style_context_add_provider(gtk_widget_get_style_context(text2), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_label_set_justify(GTK_LABEL(text2), GTK_JUSTIFY_CENTER);
    FontStyle style;
    style.setColor(0xffff, 0xffff, 0xffff);
    style.setBold(TRUE);
    style.setSize(16);
    setLabelAttributeEx(GTK_LABEL(text2), &style);

    gtk_box_pack_start(GTK_BOX(childBox), text2, TRUE, FALSE, 8);
    gtk_widget_set_halign(text2, GTK_ALIGN_CENTER);

    GtkWidget* text3 = gtk_label_new(_("The Gooroom Platform Online site allows you to see the various stories of the Gooroom"));
    gtk_widget_set_name(text3, "endpage_text3");
    gtk_style_context_add_provider(gtk_widget_get_style_context(text3), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_label_set_justify(GTK_LABEL(text3), GTK_JUSTIFY_CENTER);

    gtk_box_pack_start(GTK_BOX(childBox), text3, TRUE, FALSE, 3);
    gtk_widget_set_halign(text3, GTK_ALIGN_CENTER);
}

void createGtkView()
{
    createEndPage();
    createBeginPage();
}

void activate(GtkApplication* app)
{
    GtkWidget* box;
    GError* err = NULL;

    std::string strName = "/kr/gooroom/ui/guide.ui";
    GtkBuilder* builder = gtk_builder_new_from_resource(strName.c_str());

    window = (GtkWidget*)gtk_builder_get_object(builder, "guide-window");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    //g_signal_connect(window, "destroy", G_CALLBACK(closeWindow), NULL);

    headerBar = gtk_header_bar_new();

    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerBar), TRUE);

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    btnUndo = gtk_button_new_with_label("<");
    GtkCssProvider* cssp = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(cssp, "/kr/gooroom/css/titlebar.css");
    gtk_style_context_add_provider(gtk_widget_get_style_context(btnUndo), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);

    btnRedo = gtk_button_new_with_label(">");
    gtk_style_context_add_provider(gtk_widget_get_style_context(btnRedo), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_widget_set_sensitive(btnUndo, FALSE);
    gtk_widget_set_sensitive(btnRedo, FALSE);

    g_signal_connect(btnUndo, "clicked", G_CALLBACK(clickedUndo), NULL);
    g_signal_connect(btnRedo, "clicked", G_CALLBACK(clickedRedo), NULL);

    gtk_box_pack_start(GTK_BOX(box), btnUndo, FALSE, FALSE, 3);
    gtk_box_pack_end(GTK_BOX(box), btnRedo, TRUE, FALSE, 3);

    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_name(separator, "separator");
    gtk_style_context_add_provider(gtk_widget_get_style_context(separator), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_box_pack_end(GTK_BOX(box), separator, FALSE, FALSE, 3);

    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerBar), box);

    gtk_window_set_titlebar(GTK_WINDOW(window), headerBar);

    alignment = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
    gtk_container_add(GTK_CONTAINER(window), alignment);

    createGtkView();
    createWebView();

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);

    gtk_application_add_window(app, GTK_WINDOW(window));
    gtk_widget_show_all(GTK_WIDGET(window));

    g_object_unref(builder);
}

int main(int argc, char **argv)
{
    if (NULL != argv[1])
    {
        if (0 == strcmp(argv[1], "autostart"))
        {
            std::string expand = getExpanduser();
            expand.append("/norun");

            if (g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
                return 0;
        }
    }

    GtkApplication *app;

    app = gtk_application_new("org.gooroom.guide", G_APPLICATION_NON_UNIQUE);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    g_application_run(G_APPLICATION(app), 0, &argv[0]);

    printf("= = =>end\n");

    return 1;
}
