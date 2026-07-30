/*
 * Plain standard-I/O backend for scripted use.
 *
 * This implementation intentionally provides no terminal control, command
 * editing, colours, text attributes, pagination, or exit prompt. Commands are
 * read as newline-delimited text from stdin and game text is written to stdout.
 */

#include "ztypes.h"

static int current_row = 1;
static int current_col = 1;
static int saved_row = 1;
static int saved_col = 1;
static int cursor_saved = OFF;

void initialize_screen( void )
{
   if ( screen_cols == 0 )
      screen_cols = DEFAULT_COLS;
   if ( screen_rows == 0 )
      screen_rows = DEFAULT_ROWS;

   h_interpreter = INTERP_UNIX;
   JTERP = INTERP_UNIX;
   interp_initialized = 1;
}

void restart_screen( void )
{
   zbyte_t config;

   cursor_saved = OFF;
   config = get_byte( H_CONFIG );

   if ( h_type < V4 )
      config |= CONFIG_WINDOWS;
   else
   {
      config &= ~( CONFIG_BOLDFACE | CONFIG_EMPHASIS | CONFIG_TIMEDINPUT | CONFIG_COLOUR );
      config |= CONFIG_FIXED;
   }

   set_byte( H_CONFIG, config );
   set_word( H_FLAGS, get_word( H_FLAGS ) & ~GRAPHICS_FLAG );
}

void reset_screen( void )
{
   fflush( stdout );
   interp_initialized = 0;
}

void clear_screen( void )
{
   current_row = 1;
   current_col = 1;
}

void clear_line( void )
{
}

void clear_text_window( void )
{
}

void clear_status_window( void )
{
}

void create_status_window( void )
{
}

void delete_status_window( void )
{
}

void select_status_window( void )
{
   save_cursor_position(  );
}

void select_text_window( void )
{
   restore_cursor_position(  );
}

void move_cursor( int row, int col )
{
   current_row = row;
   current_col = col;
}

void get_cursor_position( int *row, int *col )
{
   *row = current_row;
   *col = current_col;
}

void save_cursor_position( void )
{
   if ( cursor_saved == OFF )
   {
      saved_row = current_row;
      saved_col = current_col;
      cursor_saved = ON;
   }
}

void restore_cursor_position( void )
{
   if ( cursor_saved == ON )
   {
      current_row = saved_row;
      current_col = saved_col;
      cursor_saved = OFF;
   }
}

void set_attribute( int attribute )
{
   UNUSEDVAR( attribute );
}

void display_char( int c )
{
   putchar( c );

   if ( c == '\n' )
   {
      current_row++;
      current_col = 1;
   }
   else
   {
      current_col++;
   }
}

void scroll_line( void )
{
   putchar( '\n' );
   current_row++;
   current_col = 1;
}

int input_line( int buflen, char *buffer, int timeout, int *read_size )
{
   int c;

   UNUSEDVAR( timeout );
   fflush( stdout );

   while ( ( c = getchar(  ) ) != EOF )
   {
      if ( c == '\n' || c == '\r' )
      {
         if ( c == '\r' )
         {
            c = getchar(  );
            if ( c != '\n' && c != EOF )
               ungetc( c, stdin );
         }
         return '\n';
      }

      if ( *read_size < buflen )
         buffer[( *read_size )++] = ( char ) c;
   }

   return ( *read_size > 0 ) ? '\n' : -1;
}

int input_character( int timeout )
{
   int c;

   UNUSEDVAR( timeout );
   fflush( stdout );
   c = getchar(  );

   if ( c == '\n' )
      return '\r';
   return c;
}

int codes_to_text( int c, char *s )
{
   if ( c > 154 && c < 224 )
   {
      s[0] = zscii2latin1[c - 155];

      if ( c == 220 )
      {
         s[1] = 'e';
         s[2] = '\0';
      }
      else if ( c == 221 )
      {
         s[1] = 'E';
         s[2] = '\0';
      }
      else
      {
         s[1] = '\0';
      }

      return 0;
   }

   return 1;
}
