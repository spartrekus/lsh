
/////////////////////////////////////////////////////////
// Minimal Command Line, Unix like with modfications SH 
/////////////////////////////////////////////////////////
#include <stdio.h>
#define PATH_MAX 2500
#if defined(__linux__) //linux
#define MYOS 1
#elif defined(_WIN32)
#define MYOS 2
#elif defined(_WIN64)
#define MYOS 3
#elif defined(__unix__) 
#define MYOS 4  // freebsd
#define PATH_MAX 2500
#else
#define MYOS 0
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>

#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>  


int app_debug = 3;


#define ESC "\033"
#define home() 			printf(ESC "[H") //Move cursor to the indicated row, column (origin at 1,1)
#define clrscr()		printf(ESC "[2J") //clear the screen, move to (1,1)
#define gotoxy(x,y)		printf(ESC "[%d;%dH", y, x);
#define ansigotoyx(y,x)		printf(ESC "[%d;%dH", y, x);


///  norm
#define KRED  "\x1B[31m"
#define KGRE  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KNRM  "\x1B[0m"
// higher colors ri
#define KBGRE  "\x1B[92m"
#define KBYEL  "\x1B[93m"

#define KRESET  "\x1B[0m"
#define KREV    "\x1B[7m"


int rows, cols ; 
int keys[10];
char strmsglast[PATH_MAX];
char strmsg[PATH_MAX];
int curs_posx = 0; 
int curs_posy = 0; 


static struct termios oldt;

void restore_terminal_settings(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void enable_waiting_for_enter(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void disable_waiting_for_enter(void)
{
    struct termios newt;

    /* Make terminal read 1 char at a time */
    tcgetattr(0, &oldt);  /* Save terminal settings */
    newt = oldt;  /* Init new settings */
    newt.c_lflag &= ~(ICANON | ECHO);  /* Change settings */
    tcsetattr(0, TCSANOW, &newt);  /* Apply settings */
    atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}











/////////////
char *strcomplete( const char *str )
{  
      // right side to finish
      char ptr[ 4 * strlen( str )+1];
      int strposmax = strlen( str );
      int lastposchar = strposmax;
      int firstposchar = 0;
      int i,j=0;
      int foundspace = 1;

      /// find last char
      foundspace = 1;
      for( i= strposmax-1 ; i >= 0 ; i--)
      {
	 // find where to space
         if ( foundspace == 1 ) 
         if ( str[i] == ' ' ) 
   	    lastposchar = i-1;

         if ( str[i] != ' ' ) 
           foundspace = 0;
      } 

      /// find first char
      foundspace = 1;
      for( i= 0 ; i <= lastposchar ; i++)
      {
	 // find where to space
         if ( str[i] == ' ' ) 
   	    firstposchar = i+1;
      } 

      // add the content, second part
      foundspace = 1;
      for(i= firstposchar ; i <= lastposchar ; i++)
      {
         if ( foundspace == 1 ) 
         if ( ( str[i] != ' ' )  && ( str[i] != 9 ) ) //added! with 9 for a TAB!!
          foundspace = 0;

        if ( foundspace == 0 ) 
           ptr[j++]=str[i];
      } 
      ptr[j]='\0';


   char foodir[PATH_MAX];
   strncpy( foodir, ptr, PATH_MAX );

   DIR *dirp;
   struct dirent *dp;
   char strfind[PATH_MAX];
   strncpy( strfind, ptr , PATH_MAX );

   dirp = opendir( "." );
   while  ((dp = readdir( dirp )) != NULL ) 
   {
         if (  strcmp( dp->d_name, "." ) != 0 )
         if (  strcmp( dp->d_name, ".." ) != 0 )
         {     
             if  ( ( strstr( dp->d_name , foodir ) != 0 ) 
             && ( dp->d_name[0] == foodir[0] ) )
             //&& ( dp->d_name[1] == foodir[1] ) )
             {
                 printf( "<%s>\n", dp->d_name );
                 strncpy( strfind, dp->d_name , PATH_MAX );
             }     
             else 
                printf( "%s\n", dp->d_name );
         }
   }
   closedir( dirp );


      printf( "Found: (%s)\n", strfind );

      // add the content, first part
      j = 0; 
      for(i= 0 ; i <= firstposchar-1 ; i++)
      {
          ptr[j++]=str[i];
      } 
      //ptr[j++]=' ';
      for( i=0 ; i <= strlen( strfind )-1 ; i++)
          ptr[j++]=strfind[i];
      ptr[j]='\0';

      size_t siz = sizeof ptr ; 
      char *r = malloc( sizeof ptr );
      return r ? memcpy(r, ptr, siz ) : NULL;
}










char searchitem[PATH_MAX];
void listdir(const char *name, int indent)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) 
    {
        if (entry->d_type == DT_DIR) 
	{
            char path[1024];

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf( path, sizeof(path), "%s/%s", name, entry->d_name);

            listdir( path, indent + 2);
        } 
	else 
	{
	    if ( strstr( entry->d_name , searchitem ) != 0 ) 
	    {
              printf("%s/%s\n", name , entry->d_name );
	    }
        }
    }
    closedir(dir);
}






void nls()
{ 
   DIR *dirp;
   struct dirent *dp;
   dirp = opendir( "." );
   while  ((dp = readdir( dirp )) != NULL ) 
   {
         if (  strcmp( dp->d_name, "." ) != 0 )
         if (  strcmp( dp->d_name, ".." ) != 0 )
             printf( "%s\n", dp->d_name );
   }
   closedir( dirp );
}






///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
void find_item( const char *foodir )
{
   DIR *dirp;
   struct dirent *dp;
   dirp = opendir( "." );
   while  ((dp = readdir( dirp )) != NULL ) 
   {
         if (  strcmp( dp->d_name, "." ) != 0 )
         if (  strcmp( dp->d_name, ".." ) != 0 )
         {     
             if ( ( strstr( dp->d_name , foodir ) != 0 ) 
             && ( dp->d_name[0] == foodir[0] ) 
             && ( dp->d_name[1] == foodir[1] ) )
             {
                 printf( "<%s>\n", dp->d_name );
                 strncpy( strmsg , dp->d_name , PATH_MAX );
             }     
             else 
                printf( "%s\n", dp->d_name );
         }
   }
   closedir( dirp );
}









int mgetchar() 
{
   int fooch, i; 
   fooch = getchar();
   for( i = 0; i <= 9 ; i++) 
      keys[i] = keys[i+1];
   keys[ 9 ] = fooch; 
   return fooch; 
}










//////////////////////////////////////////////
//////////////////////////////////////////////
void printmsg( char *inittxt, char *foomsg ) 
{
  int i ; 

  if ( strcmp( foomsg, "" )  == 0 ) 
  {
       printf( "%s", inittxt );
       printf( "" );
  }
  else 
  {
    printf( "\33[2K" );  //clr
    printf( "\r" );      //homeline
    printf( "%s", inittxt );
    for( i = 0; i <= strlen( foomsg )-1 ; i++) 
    {
       printf( "%c", foomsg[ i ]  );
    }


  if ( curs_posx == strlen( foomsg )+1 )  
  {
     printf( "\r" );
     printf( "%s", KRESET);
     printf( "%s", inittxt );
     printf( "%s", foomsg );
  }
  else
  {

    printf( "\r" );
    printf( "%s", KRESET);
    printf( "%s", inittxt );
    for( i = 0; i <= strlen(foomsg) -1 ; i++) 
       printf( "%c", foomsg[ i ]  );

    printf( "\r" );
    printf( "%s", KRESET);
    printf( "%s", inittxt );

    for( i = 0; i <= curs_posx ; i++) 
      printf( "\x1B[1C" );

    printf( "\x1B[1D" );

  }
 }
}




void change_dir( char *foodir )
{
      char cwddir[PATH_MAX];
      chdir( foodir );
      printf( "Path: %s\n" , getcwd( cwddir, PATH_MAX ) );
}



void change_mode()
{
                 clrscr(); home(); 
                 if      ( app_debug == 0 )  app_debug = 1; 
                 else if ( app_debug == 1 )  app_debug = 2; 
                 else if ( app_debug == 2 )  app_debug = 3; 
                 else if ( app_debug == 3 )  app_debug = 4; 
                 else if ( app_debug == 4 )  app_debug = 5; 
                 else      app_debug = 0; 
                 if ( ( app_debug != 5 ) && ( app_debug != 2 ) )
                   curs_posx = strlen( strmsg ); 
}


//////////////////////////////////////////
//////////////////////////////////////////
int main( int argc, char *argv[])
{
     char clipboard[PATH_MAX];
     strncpy( clipboard, "", PATH_MAX );

     ////////////////////////////////////////////////////////
     if ( argc == 2 )
     if ( strcmp( argv[1] , "-yel" ) ==  0 )
     {
       printf("%s", KBYEL);
       printf("Hello Yellow Terminal (bright)\n");
       return 0;
     }


     //struct winsize w; // need ioctl and unistd 
     //ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );

     int  goi;  int goj = 0;  int fdsp = 0;
     char strintcmd[PATH_MAX];
     char strintarg[PATH_MAX];
     char charo[PATH_MAX];
     char ptr[PATH_MAX];
     int ch = 0; int j, chr, i ;
     disable_waiting_for_enter(); 


     int foousergam = 0;
     char cwd[PATH_MAX]; 

     for( i = 0; i <= 9 ; i++) keys[i] = 0; 
     strncpy( strmsglast, "", PATH_MAX );
     strncpy( strmsg, "", PATH_MAX );
     while( foousergam == 0 )
     {

         if ( strcmp( strmsg, "" ) == 0 ) curs_posx = 0; 
         if ( curs_posx <= 0 ) curs_posx = 0;
         if ( curs_posx >= ( strlen( strmsg )+1) )  curs_posx = strlen( strmsg );

         if ( app_debug == 199 ) 
         {
            // pro module, (non available in free demo)
         }

         else if ( app_debug == 1 ) 
         {
            home();
            printf( "\nMode 1; KEY %d %c\n", ch , ch );
            printf( "\n" );
            printf( "%s", strmsg );
         }

         else if ( app_debug == 4 ) 
         {
            printf( "\33[2K" ); 
            printf( "\r" );
            printf( "$ %s", strmsg );
         }

         else if ( app_debug == 3 ) 
         {
            printf( "\33[2K" ); 
            printf( "\r" );
            printf( "(%s) %s", getcwd( cwd, PATH_MAX), strmsg );
         }

         else if ( app_debug == 0 ) 
         {
            printf( "\n" );
            printf( "%s", strmsg );
         }

         else if ( app_debug == 5 ) 
         {
            printmsg( "*$ ", strmsg );
         }

         else if ( app_debug == 2 ) 
         {
            clrscr();
            home();
            printf( "Mode 2\n" );
            for( i = 0; i <= 9 ; i++) 
            printf( "TERM Keys: %d\n" , keys[i] ); 
            printf( "Degug mode (d): %d\n" , app_debug );
            printf( "Degug str     (S): '%s'\n" , strmsg );
            printf( "Degug strlen  (d): '%lu'\n" , (unsigned long) strlen(strmsg) );
            printf( "Curs(X,Y,MX)(S): %d %d\n" , curs_posx , curs_posy );
            printf( "Curs(C): %c\n" , strmsg[ curs_posx ] );
            printf( "\nKEY %d %c\n", ch , ch );
            printf( "\n" );
            printmsg( "*$ ", strmsg );
         }

         ch = mgetchar(); 


         if ( ch == 18 )   // CTRL+R
            change_mode();

         else if ( ch == 25 )   // CTRL+Y  Copy
         {   
            strncpy( clipboard, strmsg , PATH_MAX );
         }
         else if ( ch == 16 )   // CTRL+P  Paste
         {   
            strncpy( strmsg , clipboard, PATH_MAX );
            curs_posx = strlen( strmsg );
         }

         else if ( ch == 23 )   // CTRL+W 
         {   
           strncat( strmsg , ">" , PATH_MAX - strlen( strmsg ) -1 );
           curs_posx = strlen( strmsg );
         }
         else if ( ch == 5 )    // CTRL+E 
         {   
           strncat( strmsg , "=" , PATH_MAX - strlen( strmsg ) -1 );
           curs_posx = strlen( strmsg );
         }

         else if ( ch == 15 )   // CTRL+O to change dir with argument
         {
            printf( "\n" );
            change_dir( strmsg );
            strncpy( strmsg, "", PATH_MAX );
         }

         else if ( ch == 9 )   // TAB
         {
           if ( strmsg[ strlen( strmsg ) -1 ] != ' ' )
           {
            if ( strstr( strmsg , " " ) != 0 ) 
            { 
              strncpy( strmsg, strcomplete( strmsg ), PATH_MAX );
              curs_posx = strlen( strmsg );
            }
            else 
            { 
              find_item(  strmsg   ); 
              curs_posx = strlen( strmsg );
            }
            printf( "\n" );
           }
         }

         else if ( ch == 6 )   // CTRL+F to find argument
         {
            strncpy( searchitem, strmsg , PATH_MAX );          
            printf( "\n" );
            listdir( ".", 0 );
            printf( "\n" );
         }


         else if ( ch == 10 )
         {   
           if ( strcmp( strmsg, "" ) == 0 ) 
           {
              printf( "\n" );
           }
           else if ( strcmp( strmsg, "" ) != 0 ) 
           {

             fdsp = 1; goj = 0; 
             for( goi = 0 ;  ( goi <= strlen(strmsg)-1 ) && ( strmsg[ goi ] != ' ' ) ; goi++)
                  strintcmd[goj++]=strmsg[goi];
             strintcmd[goj]='\0';

             fdsp = 0; goj = 0; 
             for( goi = 0 ;  ( goi <= strlen(strmsg)-1 ) ; goi++)
             {
               if ( fdsp == 1 ) strintarg[goj++]=strmsg[goi];
               if ( strmsg[ goi ] == ' ' )  fdsp = 1;
             }
             strintarg[goj]='\0';


             if ( strcmp( strmsg , "cd .." ) == 0 ) 
	     { 
                  printf( "\n" );
                  change_dir( ".." );
                  printf( "\n" );
                  strncpy( strmsg, "", PATH_MAX );
	     } 
             else if ( strcmp( strmsg , "cd" ) == 0 ) 
	     { 
                  printf( "\n" );
                  change_dir( getenv( "HOME" ) );
                  printf( "\n" );
                  strncpy( strmsg, "", PATH_MAX );
	     } 

             else if ( strcmp( strintcmd , "cd" ) == 0 ) 
	     { 
                  printf( "\n  (cd to %s..)\n",  strintarg );
	          change_dir( strintarg );
	          printf( "\n" );
	          strncpy( strmsg, ""  ,  PATH_MAX ); 
	     } 

             else if ( strcmp( strmsg , "exit" ) == 0 ) 
	     {     strncpy( strmsg, ""  ,  PATH_MAX );   foousergam = 1;   } 

             else if ( strcmp( strmsg , "!exit" ) == 0 ) 
	     {     strncpy( strmsg, ""  ,  PATH_MAX );   foousergam = 1;   } 


             else if ( strcmp( strmsg , "!compile" ) == 0 ) 
                  strncpy( strmsg,  " ./configure ; make "  , PATH_MAX );

             else if ( strcmp( strmsg , "!get wifi" ) == 0 ) 
	     { 
                  printf( "The NetBSD command line for wifi.\n" );
                  strncpy( strmsg,  " ifconfig urtwn0 up  ; ifconfig urtwn0 list scan ; /usr/sbin/wpa_supplicant -B -D bsd -i urtwn0 -c /etc/wpa_supplicant.conf ; dhclient urtwn0 "  , PATH_MAX );
	     } 

             else if ( strcmp( strmsg , "!reboot" ) == 0 ) 
                  strncpy( strmsg,  " reboot "  , PATH_MAX );


             else if ( strcmp( strmsg , "!cat" ) == 0 ) 
             {     
                  clrscr(); home();  
                  printf( "= Menu Cat =\n" ); 
                  strncpy( strmsg,  ""  , PATH_MAX );
                  getchar(); 
             }

             else if ( strcmp( strmsg , "!clr" ) == 0 ) 
             {     clrscr(); home();  }

             else if ( strcmp( strmsg , "!help" ) == 0 ) 
	     { 
                  clrscr(); home();
	          printf( "\n" ); 
                  printf( "===============================\n" );
                  printf( "=== HELP LSH (C) GNU Spart. ===\n" );
                  printf( "===============================\n" );
	          printf( "\n" ); 
	          printf( "- Print the command and run it with system().\n" ); 
	          printf( "- !help : this help.\n" );
	          printf( "- !exit : Exit this app.\n" );
	          printf( "- !ls :  internal ls within this app.\n" );
	          printf( "- F1 or CTRL+R : change view (debug) mode.\n" );
	          printf( "\n" ); 
                  printf( "===============================\n" );
	          printf( "\n" ); 
	          printf( "<Press Key>\n" ); 
	          getchar();
	     } 

             else if ( strcmp( strmsg , "!pwd" ) == 0 ) 
	     { 
                  printf( "\n" );
                  printf( "%s\n", getcwd( cwd, PATH_MAX ));
                  printf( "\n" );
                  strncpy( strmsg, "", PATH_MAX );
	     } 

             else if ( strcmp( strmsg , "!clr" ) == 0 ) 
	     { 
                  clrscr(); home();
                  strncpy( strmsg, "", PATH_MAX );
	     } 

             else if ( ( strcmp( strmsg , "!ls" ) == 0 ) ||  ( strcmp( strmsg , "!dir" ) == 0 ) )
	     {    
	       printf( "\n" ); 
               nls();  
               strncpy( strmsg, "", PATH_MAX );
	     }    
	     else 
             { 
	        printf( "\n" ); 

                if      ( app_debug == 1 ) printf( "=========================\n" );
                else if ( app_debug == 2 ) printf( "=========================\n" );
                else printf( "\n" );

                if ( strcmp( strmsg, "" ) != 0 ) 
                     system( strmsg ); 

                if ( strcmp( strmsg, "" ) != 0 ) 
                     strncpy( strmsglast, strmsg, PATH_MAX );

	        strncpy( strmsg, ""  ,  PATH_MAX ); 

                if      ( app_debug == 1 ) { printf( "\n<Press Key>\n" ); getchar(); }   
                else if ( app_debug == 2 ) { printf( "\n<Press Key>\n" ); getchar(); }   
                else if ( app_debug == 3 ) { printf( "\n" ); }
	     } 
            }
         }


         else if ( ch == 4 )
         {  
            strncpy( strmsg, ""  ,  PATH_MAX );   
            foousergam = 1;  
         } 


         else if ( ch == 27 )
         {  // 27 79 82 F3
           if ( keys[ 8 ]  == 27 )
           { clrscr(); home(); }

           // 91
           else
           {
            ch = mgetchar();
            if ( ch == 91 )
            {
               ch = mgetchar();
               if      ( ch == 65 )   { strncpy( strmsg, strmsglast,  PATH_MAX );   // A 
                   curs_posx = strlen( strmsg ); }
               else if ( ch == 66 )   { strncpy( strmsg, "" ,  PATH_MAX );          // B 
                   curs_posx = strlen( strmsg ); }
               else if ( ch == 68 )    curs_posx--; 
               else if ( ch == 67 )    curs_posx++; 
            }

            // 79 
            else if ( ch == 79 )
            {
               ch = mgetchar();
               if ( ch == 80 )      //F1
                  change_mode(); 

               else if ( ch == 81 ) //F2
	       { 
                   curs_posx = strlen( strmsg );
               }

               else if ( ch == 82 ) //F3
	       {   strncpy( strmsg, strmsglast ,  PATH_MAX );   }

               else if ( ch == 83 ) //F4
	       { 
                   curs_posx = strlen( strmsg );
               }
            }
           }
         }


	 else if ( ch == 2 ) 
	      strncpy( strmsg, ""  ,  PATH_MAX );

	 else if ( ch == 4 ) 
	 {      
            snprintf( charo, PATH_MAX , "%s%d",  strmsg, (int)time(NULL));
	    strncpy( strmsg,  charo ,  PATH_MAX );
         }

	 else if ( ( ch == 8 )  || ( ch == 127 ) )  
         {
            if ( strlen( strmsg ) >= 2 ) 
            {
              j = 0; strncpy(  ptr , "" ,  PATH_MAX );
              for ( chr = 0 ;  chr <= strlen( strmsg )-2 ; chr++) 
              {
                 ptr[j++] = strmsg[chr];
              }
	      strncpy( strmsg, ptr  ,  PATH_MAX );
            }
            else
	      strncpy( strmsg, ""  ,  PATH_MAX );
            
            curs_posx = strlen( strmsg );
         }

	 else 
	 {      
              strmsg[ curs_posx ] = ch; 
              curs_posx++;
         }
     }


     printf( "\n" );

     return 0; 
}



