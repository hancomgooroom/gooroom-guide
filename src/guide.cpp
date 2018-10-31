#include "config.h"
#include "stdafx.h"

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gdk/gdk.h>
#include <webkit2/webkit2.h>
#include <glib/gi18n-lib.h>

static std::string st;
char* test = NULL;
#define fopen_s(fp, fmt, mode)	*(fp)=fopen((fmt), (mode))

// for label
typedef struct _FontStyle {
    char* family;
    GdkColor color;
    int size;

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
}FontStyle;


static GtkWidget* window = NULL;
//static GtkBuilder* builder;
static GtkWidget* btnPrev, *btnNext;
static GtkWidget* checkBox = NULL;
static GtkWidget* alignment = NULL;

// gtk view
static GtkWidget* beginPage = NULL;
static GtkWidget* endPage = NULL;

// web view
static GtkWidget* webViewPage;
static WebKitWebView* webView = NULL;
static std::vector<std::string> filePaths = std::vector<std::string>();
static std::string curUri;

// page navi
static int pageCnt = 0;
static int pageSize = 0;

static gboolean isShowAtBegin = TRUE;

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
void closeWindow(GtkWidget* widget, gpointer* data);
void clickedPrev(GtkButton* button, gpointer data);
void clickedNext(GtkButton* button, gpointer data);
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

    //printf("= = = = => expand: %s\n", expand.c_str());
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

    pageSize = toc.size() + 1;

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

void clickedPrev(GtkButton* button, gpointer userData)
{
    pageCnt--;
    if (0 == pageCnt)
    {
        gtk_widget_set_sensitive(btnPrev, FALSE);
        changePageView(webViewPage, beginPage);
    }
    else
    {
        if (pageSize == pageCnt + 1)
        {
            gtk_widget_set_sensitive(btnNext, TRUE);
            changePageView(endPage, webViewPage);

            return;
        }

        loadUri(pageCnt - 1);
    }
}

void clickedNext(GtkButton* button, gpointer userData)
{
    pageCnt++;
    if (1 == pageCnt)
    {
        gtk_widget_set_sensitive(btnPrev, TRUE);
        changePageView(beginPage, webViewPage);
    }
    else
    {
        if (pageSize == pageCnt)
        {
            gtk_widget_set_sensitive(btnNext, FALSE);
            changePageView(webViewPage, endPage);
            return;
        }

        loadUri(pageCnt - 1);
    }
}

void checkShowAtBegin(GtkButton* button, gpointer userData)
{
    isShowAtBegin = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
}

void closeWindow(GtkWidget* widget, gpointer* data)
{
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
            //gtk_widget_set_sensitive(btnPrev, webkit_web_view_can_go_back(webView));
            //gtk_widget_set_sensitive(btnNext, webkit_web_view_can_go_forward(webView));
        {
            const gchar* uri = webkit_web_view_get_uri(webView);

            // 데스크탑 바로가기
            if (0 == strcmp(uri, filePaths[1].c_str()) && 0 != curUri.compare(uri))
            {
                pageCnt = 2;
            }
            // 보안프레임워크 바로가기
            else if (0 == strcmp(uri, filePaths[10].c_str()) && 0 != curUri.compare(uri))
            {
                pageCnt = 11;
            }
            // 메뉴 바로가기
            else if (0 == strcmp(uri, filePaths[0].c_str()) && !curUri.empty() &&
             0 != strcmp(uri, curUri.c_str()))
            {
                pageCnt = 1;
            }
            // 시작화면 바로가기
            else if (0 == curUri.compare(uri) && 0 != curUri.compare(filePaths[0].c_str()))
            {
                //printf("= = = = =>go to begin page \n");
                uri = filePaths[0].c_str();
                gtk_widget_set_sensitive(btnPrev, FALSE);
                loadUri(0);

                pageCnt = 0;

                changePageView(webViewPage, beginPage);
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

    GdkRGBA rgba;

//    rgba.red = 0.949;
//    rgba.green = 0;//0.949;
//    rgba.blue = 0;//0.949;
//    rgba.alpha = 1;
//    webkit_web_view_set_background_color(webView, &rgba);
    gtk_container_add(GTK_CONTAINER(webViewPage), GTK_WIDGET(webView));

    g_signal_connect(webView, "load-changed", G_CALLBACK(changedLoadWebPage), NULL);

    loadUri(0);
    gtk_widget_grab_focus(GTK_WIDGET(webView));
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
        atFont = pango_attr_font_desc_new(df);
    }

    atColor = pango_attr_foreground_new(style->color.red, style->color.green, style->color.blue);

    pango_attr_list_insert(attrList, atFont);
    pango_attr_list_insert(attrList, atColor);

    gtk_label_set_attributes(label, attrList);
    pango_attr_list_unref(attrList);
}

void createBeginPage()
{
    beginPage = gtk_layout_new(NULL, NULL);

    GtkWidget* image = gtk_image_new_from_resource("/kr/gooroom/images/bg_start.jpg");
    gtk_layout_put(GTK_LAYOUT(beginPage), image, 0, 0);

    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    gint width, height;
    gtk_window_get_size(GTK_WINDOW(window), &width, &height);

    pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget* logo = gtk_image_new_from_resource("/kr/gooroom/images/logo_start.png");
    GdkPixbuf* logoBuf = gtk_image_get_pixbuf(GTK_IMAGE(logo));
    gtk_box_pack_start(GTK_BOX(box), logo, FALSE, FALSE, 30);

    gint lw = gdk_pixbuf_get_width(logoBuf);
    gint lh = gdk_pixbuf_get_height(logoBuf);

    gdouble logo_xp = (width - lw) / 3;
    gdouble logo_yp = lh / 2 + 10;
    gtk_layout_put(GTK_LAYOUT(beginPage), box, logo_xp, logo_yp);

    GtkWidget* text1 = gtk_label_new(_("Thanks you for choosing Gooroom"));
    FontStyle style;
    style.setColor(0xffff, 0xffff, 0xffff);
    style.setSize(18);
    setLabelAttributeEx(GTK_LABEL(text1), &style);
    gtk_box_pack_start(GTK_BOX(box), text1, TRUE, FALSE, 8);

    GtkWidget* text2 = gtk_label_new(_("The cloud platform promises you a pleasant PC environment with lightweight and fast performance.\n The button on the top lets you see the basic usage environment of the cloud platform"));
    gtk_label_set_justify(GTK_LABEL(text2), GTK_JUSTIFY_CENTER);
    style.setSize(14);
    setLabelAttributeEx(GTK_LABEL(text2), &style);
    gtk_box_pack_start(GTK_BOX(box), text2, TRUE, FALSE, 8);

    checkBox = gtk_check_button_new();
    GtkWidget* checkLabel = gtk_label_new(_("Show program at Startup"));
    style.setSize(12);
    setLabelAttributeEx(GTK_LABEL(checkLabel), &style);
    gtk_container_add(GTK_CONTAINER(checkBox), checkLabel);

    gtk_box_pack_start(GTK_BOX(box), checkBox, TRUE, FALSE, 10);
    gtk_widget_set_halign(checkBox, GTK_ALIGN_CENTER);

    g_signal_connect(checkBox, "toggled", G_CALLBACK(checkShowAtBegin), NULL);

    std::string expand = getExpanduser();
    expand.append("/norun");
    if (!g_file_test(expand.c_str(), G_FILE_TEST_EXISTS))
        isShowAtBegin = TRUE;
    else
        isShowAtBegin = FALSE;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkBox), isShowAtBegin);
    gtk_container_add(GTK_CONTAINER(alignment), beginPage);
}

void createEndPage()
{
    endPage = gtk_layout_new(NULL, NULL);

    GtkWidget* image = gtk_image_new_from_resource("/kr/gooroom/images/bg_start.jpg");
    gtk_layout_put(GTK_LAYOUT(endPage), image, 0, 0);

    GdkPixbuf* pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    gint width, height;
    gtk_window_get_size(GTK_WINDOW(window), &width, &height);

    pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_layout_put(GTK_LAYOUT(endPage), box, 140, 140);

    GtkWidget* text1 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(text1), _("<b>You can begin the Gooroom platform</b>"));
    FontStyle style;
    style.setColor(0xffff, 0xffff, 0xffff);
    style.setSize(30);
    setLabelAttributeEx(GTK_LABEL(text1), &style);
    gtk_box_pack_start(GTK_BOX(box), text1, TRUE, FALSE, 7);

    GtkWidget* text2 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(text2), _("Website : <a href=\"https://www.gooroom.kr/\">https://www.gooroom.kr/</a>"));
    gtk_label_set_justify(GTK_LABEL(text2), GTK_JUSTIFY_CENTER);
    style.setSize(18);
    setLabelAttributeEx(GTK_LABEL(text2), &style);
    gtk_box_pack_start(GTK_BOX(box), text2, TRUE, FALSE, 20);

    GtkWidget* text3 = gtk_label_new(_("The Gooroom Platform Online site allows you to see the various stories of the Gooroom"));
    gtk_label_set_justify(GTK_LABEL(text3), GTK_JUSTIFY_CENTER);
    style.setColor(0xffff, 0xffff, 0xffff);
    style.setSize(16);
    setLabelAttributeEx(GTK_LABEL(text3), &style);
    gtk_box_pack_start(GTK_BOX(box), text3, TRUE, FALSE, 7);
}

void createGtkView()
{
    createEndPage();
    createBeginPage();
}

void activate(GtkApplication* app)
{
    GtkWidget* headerBar = NULL;
    GtkWidget* box;
    GError* err = NULL;

    std::string strName = "/kr/gooroom/ui/guide.ui";
    GtkBuilder* builder = gtk_builder_new_from_resource(strName.c_str());

    window = (GtkWidget*)gtk_builder_get_object(builder, "guide-window");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(closeWindow), NULL);

    headerBar = gtk_header_bar_new();

    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerBar), TRUE);

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    btnPrev = gtk_button_new_with_label("<");
//    GtkWidget* labelPrev = gtk_label_new("<");
//    FontStyle style;
//    style.setColor(0x33ff, 0x33ff, 0x33ff);
//    style.setSize(12);
//    setLabelAttributeEx(GTK_LABEL(prevLabel), &style);
//    gtk_container_add(GTK_CONTAINER(btnPrev), labelPrev);
    GtkCssProvider* cssp = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(cssp, "/kr/gooroom/css/titlebar.css");
    gtk_style_context_add_provider(gtk_widget_get_style_context(btnPrev), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);

    btnNext = gtk_button_new_with_label(">");
//    GtkWidget* labelNext = gtk_label_new(">");
//    setLabelAttributeEx(GTK_LABEL(nextLabel), &style);
//    gtk_container_add(GTK_CONTAINER(btnNext), labelNext);

    gtk_style_context_add_provider(gtk_widget_get_style_context(btnNext), GTK_STYLE_PROVIDER(cssp), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_widget_set_sensitive(btnPrev, FALSE);
    gtk_widget_set_sensitive(btnNext, TRUE);

    g_signal_connect(btnPrev, "clicked", G_CALLBACK(clickedPrev), NULL);
    g_signal_connect(btnNext, "clicked", G_CALLBACK(clickedNext), NULL);

    gtk_box_pack_start(GTK_BOX(box), btnPrev, FALSE, FALSE, 3);
    gtk_box_pack_end(GTK_BOX(box), btnNext, TRUE, FALSE, 3);

    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
//    GtkCssProvider* cssp = gtk_css_provider_new();
//    gtk_css_provider_load_from_resource(cssp, "/kr/gooroom/css/titlebar.css");
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
    // FIXME: home 경로에 있는 .gooroom 폴더 제거 (정식 배포 시 제거해야함)
    removeBeforeExpand();

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

    return 1;
}
