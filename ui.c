'''
	name:guoxiaowen
	time:2018-11-11
	lang:C
'''

#include<gtk/gtk.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <pthread.h>
#include "proxy.h"

  char *ip1[100]={0};
  char *www1[100]={0};
  char entrystring[200];

struct _WWW{
    char *ip[100];
    char *www[100];
    int countip;
    int countwww;
};

struct _WWW WWW = {.countip = 0,.countwww = 0};


int button_run(void *arg);


GtkWidget *window_test;


void threads(GtkWidget *widget,gpointer *data)
{
  pthread_t thread1;
  int ret_thrd1;
  ret_thrd1 = pthread_create(&thread1, NULL, (void *)&button_run, (void *)&WWW);
  pthread_join(thread1,NULL);
}

void enter_callback( GtkWidget *widget, gpointer entry ) 
{ 
  const gchar *entry_text; 
  char test[100];
  char *filterstring[100] = {0};
  char spl[] = " ";
  char *result = NULL;
  int i=0,j;
  int countip=0;
  int countwww=0;

	// 获得文本内容
  entry_text = gtk_entry_get_text(GTK_ENTRY(entry)); 
  strcpy(entrystring, entry_text);
  filterstring[0] = strtok(entrystring,spl);
  while(filterstring[i])
  {
    i++;
    filterstring[i] = strtok(NULL,spl);
  }
  j = i;
  for(i = 0;i<j;i++)
  {
    strcpy(test,filterstring[i]);
      if(test[0]>='1'&&test[0]<='9')
      {
          ip1[countip] = filterstring[i];
          countip++;
      }
      else
      {
        www1[countwww] = filterstring[i];
          countwww++;
      }
  }

  WWW.countip = countip;
  WWW.countwww = countwww;
  
  for(int k=0;k<countip;k++)
  {
      WWW.ip[k] = ip1[k];
    //   printf("%s\n",WWW.ip[k]);
  }

  for(int k=0;k<countwww;k++)
  {
      WWW.www[k] = www1[k];
    //   printf("%s\n",WWW.www[k]);
  }

}

int button_run (void *arg){

    int listenfd, *connfd, clientlen, port = 0;
    struct sockaddr_in clientaddr;
    pthread_t tid;
    struct _WWW *pstru;       
    pstru = ( struct _WWW *) arg;
    printf("%d\n",pstru->countip);
    printf("%d\n",pstru->countwww);
    for(int i=0;i<pstru->countip;i++)
    {
        ip[i] = pstru->ip[i];
    }
    for(int m=0;m<pstru->countwww;m++)
    {
        www[m] = pstru->www[m];
    }

    for(int m=0;m<pstru->countwww;m++)
    {
        printf("%s\n",www[m]);
    }
   

    // ignore SIGPIPE
    Signal(SIGPIPE, SIG_IGN);

    // check input

    port = 10000;
    if (port == 0) {
        fprintf(stderr, "please enter valid port number\n");
        exit(1);
    }

    listenfd = Open_listenfd(port);
    if (listenfd < 0) {
        fprintf(stderr, "cannot listen to port: %d\n", port);
        exit(1);
    }

    while (1) {
        clientlen = sizeof(clientaddr);
        int count = 0;
        // use MAX_MALLOC to limit max tries
        while ((connfd = (int *)malloc(sizeof(int))) == NULL) {
            if (count > MAX_MALLOC) {
                break;
            }
            count++;
            sleep(1);
        }
        *connfd = Accept(listenfd, (SA *)&clientaddr,
                         (socklen_t *)&clientlen);
        Pthread_create(&tid, NULL, (void *)job, (void *)connfd);
    }
    return 0;
}

void button_event(GtkWidget *widget, gpointer *data) /*回调函数*/

{
  exit(0);
}

void show_test_window(GtkWidget *widget, gpointer data)
{
  gtk_widget_show_all(window_test);
}

/* 用指定的参数创建一个按钮盒 */
GtkWidget *create_bbox( gint  horizontal,
                        gint  spacing,
                        gint  child_w,
                        gint  child_h,
                        gint  layout )
{
  GtkWidget *frame;
  GtkWidget *bbox;
  GtkWidget *button1;
  GtkWidget *button2;
  GtkWidget *button3;

  // frame = gtk_frame_new (title);

  if (horizontal)
    bbox = gtk_hbutton_box_new ();
  else
    bbox = gtk_vbutton_box_new ();

  gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), bbox);

  /* 设置按钮盒的外观 */
  gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), layout);
  gtk_box_set_spacing (GTK_BOX (bbox), spacing);
  /*gtk_button_box_set_child_size (GTK_BUTTON_BOX (bbox), child_w, child_h);*/

  button1 = gtk_button_new_with_label("连接");
  gtk_container_add (GTK_CONTAINER (bbox), button1);
  

  button2 = gtk_button_new_with_label("退出");
  gtk_container_add (GTK_CONTAINER (bbox), button2);
 

  button3 = gtk_button_new_with_label("帮助");
  gtk_container_add (GTK_CONTAINER (bbox), button3);
  // gtk_signal_connect(GTK_OBJECT(button3), "clicked", GTK_SIGNAL_FUNC(button_event), "clicked");
    gtk_signal_connect(GTK_OBJECT(button1), "clicked", GTK_SIGNAL_FUNC(threads),NULL);
    gtk_signal_connect(GTK_OBJECT(button2), "clicked", GTK_SIGNAL_FUNC(button_event),NULL);
    g_signal_connect(G_OBJECT(button3), "clicked", G_CALLBACK(show_test_window),NULL);

  return frame;
}

GtkWidget *create_test_window()    //点击帮助按钮然后弹出帮助页面
{
  GtkWidget *text;
  GtkWidget *vbox;
  GtkWidget *image;
  GtkWidget *image_two;
  GdkPixbuf *src;
  GdkPixbuf *dest;

  window_test = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window_test),"帮助");

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window_test), vbox);
  gtk_window_set_default_size(GTK_WINDOW(window_test), 400, 300);

  // image = get_image_new_from_file("1.jpg");
  // gtk_container_add(GTK_CONTAINER(vbox),image);

  src = gdk_pixbuf_new_from_file("1.jpg",NULL);
  dest = gdk_pixbuf_scale_simple(src,100,100,GDK_INTERP_BILINEAR);
  image_two = gtk_image_new_from_pixbuf(dest);
  g_object_unref(src);
  g_object_unref(dest);
  gtk_container_add(GTK_CONTAINER(vbox), image_two);


  text = gtk_label_new("  添加URL：直接添加链接或者ip,中间使用空间进行隔离"\
                       "  过滤多个url的时候，回车换行输入多个\n"
                       "  编译方式：gcc -g -Wall -o mp csapp.c cache.c proxy.c -lpthread\n"
                       "  支持平台：在Ubuntu下可以运行\n"
                       "  协议支持：支持http协议GET、POST方法，带缓存\n");
  gtk_container_add(GTK_CONTAINER(vbox), text);


  return window_test;
}

int main(int argc, char const *argv[])
  {
    static GtkWidget *window = NULL;
    GtkWidget *main_vbox;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *entry;
    GtkWidget *label;
    GtkWidget *frame_horz;
    GtkWidget *frame_vert;
    GtkWidget *frame_addurl;

    /* 初始化 */
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Proxy");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 500);

    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gtk_main_quit),
                     NULL);
    g_signal_connect(G_OBJECT(window_test), "destroy", G_CALLBACK(gtk_main_quit),
                     NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER_ALWAYS);

    main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    frame_vert = gtk_frame_new("版本信息");
    gtk_box_pack_start(GTK_BOX(main_vbox), frame_vert, TRUE, TRUE, 10);
    label = gtk_label_new("开发实践：2018-11-30\n"
                          "开发人员：501组合\n"
                          "版本信息：V-1.0.0\n"
                          "反馈：QQ:123455\n");
    gtk_container_add(GTK_CONTAINER(frame_vert), label);

    frame_addurl = gtk_frame_new("添加过滤的url");
    gtk_box_pack_start(GTK_BOX(main_vbox), frame_addurl, TRUE, TRUE, 10);
    entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1000);
    gtk_entry_set_text(GTK_ENTRY(entry), "192.168.135.132");
    gtk_container_add(GTK_CONTAINER(frame_addurl), entry);
    /* 如果我们想在用户输入文本时进行响应，可以为activate设置回调函数。
	 * 当用户在文本输入构件内部按回车键时引发Activate信号；
	 */
    g_signal_connect(entry, "activate", G_CALLBACK(enter_callback), entry);

    frame_horz = gtk_frame_new("");
    gtk_box_pack_start(GTK_BOX(main_vbox), frame_horz, TRUE, TRUE, 10);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(frame_horz), vbox);

    gtk_box_pack_start(GTK_BOX(vbox),
                       create_bbox(TRUE, 40, 85, 20, GTK_BUTTONBOX_SPREAD),
                       TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    window_test = create_test_window();

    /* 进入事件循环 */
    gtk_main();

    return 0;
  }
