#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <gtk/gtk.h>

int sock1; 
int sock2; 
pthread_t threads[2];

typedef struct
{
     GtkWidget *entry, *textview;
} Widgets;

static void insert_text (GtkButton*, Widgets*);
static void insert_text2(Widgets*,char*);
static void sfile(Widgets*);
Widgets *w,*x;
char* text3;
char* name;

void nname(GtkButton *button){

	char* text;	
    text = gtk_entry_get_text (GTK_ENTRY (x->entry));
	strcat(text,":");
	name = text;
	printf("%s",name);
	gtk_main_quit();
	
}

int set_name(int argc, char** argv){
	GtkWidget *window,*hbox,*vbox;
    GtkWidget *button;

    x = g_slice_new (Widgets);
    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	button = gtk_button_new_with_label ("Set name");	
	x->entry = gtk_entry_new ();
	g_signal_connect (G_OBJECT (button), "clicked",
                       G_CALLBACK (nname), NULL);
	
	hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start_defaults (GTK_BOX (hbox), x->entry);
    gtk_box_pack_start_defaults (GTK_BOX (hbox), button);

	vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show_all (window);
    gtk_main ();
	return 0;
}




void store_filename (GtkWidget *widget, gpointer user_data) {
   GtkWidget *file_selector = GTK_WIDGET (user_data);
   const gchar *selected_filename;

   selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
   g_print ("Selected filename: %s\n", selected_filename);
}
   
static void sfile(Widgets *w)
{
	GtkWidget *file_selector;
   
   file_selector = gtk_file_selection_new ("Please select a file for editing.");
   
   g_signal_connect (GTK_FILE_SELECTION (file_selector)->ok_button,
                     "clicked",
                     G_CALLBACK (store_filename),
                     file_selector);
   
   g_signal_connect_swapped (GTK_FILE_SELECTION (file_selector)->ok_button,
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy), 
                             file_selector);

   g_signal_connect_swapped (GTK_FILE_SELECTION (file_selector)->cancel_button,
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             file_selector); 
   
   gtk_widget_show (file_selector);
}

static void insert_text (GtkButton *button,Widgets *w)
{	
     GtkTextBuffer *buffer;
     GtkTextMark *mark;
     GtkTextIter iter;
     char* text;
	 char* tmp;

     buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w->textview));
     text = gtk_entry_get_text (GTK_ENTRY (w->entry));

     mark = gtk_text_buffer_get_insert (buffer);
     gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

	if (gtk_text_buffer_get_char_count(buffer))
    gtk_text_buffer_insert (buffer, &iter, "\n", 1);
     gtk_text_buffer_insert (buffer, &iter, text, -1);

	tmp = malloc(strlen(name)+1+4); /* make space for the new string (should check the return value ...) */
	strcpy(tmp, name); /* copy name into the new var */
	strcat(tmp, text); /* add the extension */	
	send(sock1, tmp, strlen(tmp)+1, 0);
}
static void insert_text2(Widgets *w,char* text2)
{
     GtkTextBuffer *buffer;
     GtkTextMark *mark;
     GtkTextIter iter;

     buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w->textview));

     mark = gtk_text_buffer_get_insert (buffer);
     gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

	if (gtk_text_buffer_get_char_count(buffer))
    gtk_text_buffer_insert (buffer, &iter, "\n", 1);
     gtk_text_buffer_insert (buffer, &iter, text2, -1);

}

int connect_cli(char* port){
	char* ip="10.5.99.51";
	int sockfd, portno, n;
    	struct sockaddr_in serv_addr;
    	struct hostent *server;

    	char buffer[256];
   	portno = atoi(port);
   	sockfd = socket(AF_INET, SOCK_STREAM, 0);
   	if (sockfd < 0) 
   	 error("ERROR opening socket");
  	server = gethostbyname(ip);
  	if (server == NULL) {
   	   fprintf(stderr,"ERROR, no such host\n");
   	   exit(0);
  	}
    	bzero((char *) &serv_addr, sizeof(serv_addr));
   	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    	serv_addr.sin_port = htons(portno);
    	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        	error("ERROR connecting");
	bzero(buffer,256);
    	n = read(sockfd,buffer,255);
	printf("%s\n",buffer);

	return sockfd;

}

void *hilowrite(void *arg){
	//char buffer[256];
	while(1){		
		//printf("texto: ");
    	//	bzero(buffer,256);
    	//	fgets(buffer,255,stdin);
		//send(sock1, buffer, strlen(buffer)+1, 0);	
	}
}
void *hiloread(void *arg){
	char buffer[256];
	while(1){	
		recv(sock2, buffer, 255, 0);
		printf("Message: %s \n",buffer);
		insert_text2((gpointer)w,buffer);
		fflush(stdout);	
	}

}


int main(int argc, char** argv)
{
	char* port1="5050";
	char* port2="5051";
	
	
	sock1=connect_cli(port1);
	sock2=connect_cli(port2);

	printf("Conectado al Servidor\n");

	set_name(argc,argv);	

	pthread_create(&threads[0], NULL, hiloread, "...");
	fflush(stdout);
	pthread_create(&threads[1], NULL, hilowrite, "...");

     GtkWidget *window, *scrolled_win, *hbox, *vbox, *insert, *sendf;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
     w = g_slice_new (Widgets);

     gtk_init (&argc, &argv);

     window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
     gtk_window_set_title (GTK_WINDOW (window), "Text Iterators");
     gtk_container_set_border_width (GTK_CONTAINER (window), 10);
     gtk_widget_set_size_request (window, -1, 200);

     w->textview = gtk_text_view_new ();
     w->entry = gtk_entry_new ();

     insert = gtk_button_new_with_label ("Insert Text");
	sendf = gtk_button_new_with_label ("Atach file");

     g_signal_connect (G_OBJECT (insert), "clicked",
               G_CALLBACK (insert_text),
               (gpointer) w);

	g_signal_connect (G_OBJECT (sendf), "clicked",
               G_CALLBACK (sfile),
               (gpointer) w);
	
     scrolled_win = gtk_scrolled_window_new (NULL, NULL);
     gtk_container_add (GTK_CONTAINER (scrolled_win), w->textview);

     hbox = gtk_hbox_new (FALSE, 5);
     gtk_box_pack_start_defaults (GTK_BOX (hbox), w->entry);
     gtk_box_pack_start_defaults (GTK_BOX (hbox), insert);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), sendf);

     vbox = gtk_vbox_new (FALSE, 5);
     gtk_box_pack_start (GTK_BOX (vbox), scrolled_win, TRUE, TRUE, 0);
     gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

     gtk_container_add (GTK_CONTAINER (window), vbox);
     gtk_widget_show_all (window);

     gtk_main();
     return 0;

}
