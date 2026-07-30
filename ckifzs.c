
/* $Id: ckifzs.c,v 1.1.1.1 2000/05/10 14:21:34 jholder Exp $ 
 * --------------------------------------------------------------------
 * see doc/License.txt for License Information 
 * --------------------------------------------------------------------
 *
 * File name: $Id: ckifzs.c,v 1.1.1.1 2000/05/10 14:21:34 jholder Exp $
 *
 * Description:
 *
 * Modification history:
 * $Log: ckifzs.c,v $
 * Revision 1.1.1.1  2000/05/10 14:21:34  jholder
 *
 * imported
 *
 *
 * --------------------------------------------------------------------
 */

/* quick command to do a sanity check on a QUETZAL file
 *
 * 28/04/97
 */

/* Altered by John W. Kennedy  2000-03-17 :
 *  Addtnl ln of output with release number, "serial number" checksum fields
 *  Optional file buffering added
 *  Odd-size logic corrected
 *  Stack dump added
 *  Data dump added
 */

/*
#define LONG_DUMP
*/

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GOT_HEADER   0x01
#define GOT_MEMORY   0x02
#define GOT_STACKS   0x04
#define GOT_CMEM     0x10
#define GOT_UMEM     0x20
#define GOT_ALL      0x07

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif
#define EXIT_USAGE 2

typedef short ZINT16;
typedef unsigned short ZUINT16;

static unsigned char read_byte( FILE * fp )
{
   int c;

   if ( ( c = getc( fp ) ) == EOF )
   {
      printf( "*** Premature EOF.\n" );
      exit( EXIT_FAILURE );
   }
   return ( unsigned char ) c;
}

static int check_stacks( FILE *fp, uint32_t length )
{
   uint32_t remaining = length;
   uint32_t frame = 0;

   while ( remaining > 0 )
   {
      uint32_t return_pc;
      uint32_t words;
      uint16_t stack_words;
      unsigned char flags;
      unsigned char result;
      unsigned char arguments;
      unsigned int locals;
      uint32_t i;

      ++frame;
      if ( remaining < 8 )
      {
         printf( "*** Stks frame %" PRIu32 " has only %" PRIu32
               " bytes; at least 8 are required.\n", frame, remaining );
         while ( remaining-- > 0 )
            ( void ) read_byte( fp );
         return 0;
      }

      return_pc = ( uint32_t ) read_byte( fp ) << 16;
      return_pc |= ( uint32_t ) read_byte( fp ) << 8;
      return_pc |= read_byte( fp );
      flags = read_byte( fp );
      result = read_byte( fp );
      arguments = read_byte( fp );
      stack_words = ( uint16_t ) read_byte( fp ) << 8;
      stack_words |= read_byte( fp );
      remaining -= 8;

      locals = flags & 0x0Fu;
      words = locals + ( uint32_t ) stack_words;
      if ( words > remaining / 2 )
      {
         printf( "*** Stks frame %" PRIu32 " requires %" PRIu32
               " value bytes, but only %" PRIu32 " remain.\n",
               frame, words * 2, remaining );
         while ( remaining-- > 0 )
            ( void ) read_byte( fp );
         return 0;
      }

#if defined SHORT_DUMP || defined LONG_DUMP
      printf( "    PC: %6" PRIX32 "  Flags: %02X  Return: %02X"
            "  Args: %02X  Local stack use: %u\n",
            return_pc, flags, result, arguments, stack_words );
#else
      ( void ) return_pc;
      ( void ) result;
      ( void ) arguments;
#endif

      for ( i = 0; i < words; ++i )
      {
         unsigned int value = ( unsigned int ) read_byte( fp ) << 8;

         value |= read_byte( fp );
#if defined SHORT_DUMP || defined LONG_DUMP
         if ( !( i % 8 ) )
            fputs( "     ", stdout );
         printf( " %04X", value );
         if ( i % 8 == 7 || i + 1 == words )
            putchar( '\n' );
#else
         ( void ) value;
#endif
      }
      remaining -= words * 2;
   }

   return 1;
}

#if defined SHORT_DUMP || defined LONG_DUMP 

static void coredump( uint32_t offset, unsigned char uch )
{                               
   if ( !( offset % 16 ) )
      printf( "\n    %08" PRIX32 "  ", offset );
   else if ( !( offset % 8 ) )
      fputs( "  ", stdout );    
   else if ( !( offset % 4 ) )
      putchar( ' ' );           
   printf( "%02X", uch );       
}                               

#endif 

int main( int argc, char **argv )
{
   FILE *fp = stdin;
   char buffer[BUFSIZ];         
   int i;
   unsigned char status = 0;
   char id[5];
   ZUINT16 errors = 0;
   ZUINT16 warnings = 0;
   uint32_t filelen, cklen, odd_byte;

   if ( argc == 2 && strcmp( argv[1], "-" ) )
   {
      if ( ( fp = fopen( argv[1], "rb" ) ) == NULL )
      {
         perror( argv[1] );
         exit( EXIT_USAGE );
      }
      setbuf( fp, buffer );     
   }
   else
   {
      fprintf( stderr, "usage: %s [file]\n\n", argv[0] );
      fprintf( stderr, "Checks a QUETZAL/IFZS file for conformance to the standard.\n" );
      fprintf( stderr,
               "This does not do in-depth checking, but makes sure all chunk lengths are\n" );
      fprintf( stderr,
               "correct, and ensures all necessary chunks appear and in the correct order.\n" );
      exit( EXIT_USAGE );
   }

   /* print details of FORM header */
   if ( getc( fp ) != ( int ) 'F' || getc( fp ) != ( int ) 'O' || getc( fp ) != ( int ) 'R' ||
        getc( fp ) != ( int ) 'M' )
   {
      printf( "*** No FORM header: not an IFZS file.\n" );
      exit( EXIT_FAILURE );
   }

   for ( filelen = 0, i = 0; i < 4; ++i )
   {
      filelen = ( filelen << 8 ) | read_byte( fp );
   }

   if ( getc( fp ) != ( int ) 'I' || getc( fp ) != ( int ) 'F' || getc( fp ) != ( int ) 'Z' ||
        getc( fp ) != ( int ) 'S' )
   {
      printf( "*** No IFZS type: not an IFZS file.\n" );
      exit( EXIT_FAILURE );
   }

   fprintf( stdout, "FORM %" PRIu32 " IFZS\n", filelen );
   if ( filelen < 4 )
   {
      printf( "*** FORM chunk is too short to contain an IFZS type\n" );
      exit( EXIT_FAILURE );
   }
   filelen -= 4;
   if ( filelen & 1 )
   {
      printf( "*** FORM length is even in all legal files\n" );
      ++errors;
   }

   /* loop while there's still something left in the IFZS chunk */
   while ( filelen > 0 )
   {
      /* check there's enough of the IFZS chunk left */
      if ( filelen < 8 )
      {
         printf( "*** IFZS chunk too short to contain another chunk (%" PRIu32
               " bytes left)\n", filelen );
         exit( EXIT_FAILURE );
      }

      /* read this chunk's details */
      for ( i = 0; i < 4; ++i )
      {
         id[i] = read_byte( fp );
      }

      for ( i = 0; i < 4; ++i )
      {
         if ( id[i] < 0x20 || id[i] > 0x7E )
         {
            printf( "*** Illegal chunk ID: 0x%02X%02X%02X%02X\n", ( ZUINT16 ) id[0],
                    ( ZUINT16 ) id[1], ( ZUINT16 ) id[2], ( ZUINT16 ) id[3] );
            exit( EXIT_FAILURE );
         }
      }

      for ( cklen = 0, i = 0; i < 4; ++i )
      {
         cklen = ( cklen << 8 ) | read_byte( fp );
      }
      odd_byte = cklen & 1u;
      id[4] = ( unsigned char ) 0;
      printf( "  %s %6" PRIu32, id, cklen );

      if ( cklen > filelen - 8 || filelen - 8 - cklen < odd_byte )
      {
         printf( "\n*** Chunk extends past end of IFZS chunk (%" PRIu32
               " left in IFZS)\n", filelen - 8 );
         exit( EXIT_FAILURE );
      }

      /* if it's a known chunk id, print more information */
      if ( !strncmp( id, "IFhd", 4 ) )
      {
         printf( " (QUETZAL header)\n" );
         if ( status & GOT_HEADER )
         {
            printf( "*** warning: later IFhd chunk will be ignored by readers.\n" );
            ++warnings;
         }
         else
         {
            if ( status & ( GOT_STACKS | GOT_MEMORY ) )
            {
               printf( "*** IFhd must come before CMem, UMem, or Stks.\n" );
               ++errors;
            }
            status |= GOT_HEADER;
            if ( cklen < 13 )
            {
               printf( "*** IFhd chunk is too short; at least 13 bytes are required.\n" );
               ++errors;
            }
            if ( cklen >= 2 )
            {
               unsigned char uch1 = read_byte( fp );
               unsigned char uch2 = read_byte( fp );

               printf( "    Release %u", ( uch1 << 8 ) | uch2 );
               cklen -= 2;
               filelen -= 2;
               if ( cklen >= 6 )
               {
                  fputs( "  Serial number ", stdout );
                  for ( i = 0; i < 6; ++i )
                  {
                     putchar( read_byte( fp ) );
                  }
                  cklen -= 6;
                  filelen -= 6;
                  if ( cklen >= 2 )
                  {
                     uch1 = read_byte( fp );
                     uch2 = read_byte( fp );
                     printf( "  Checksum: %4X", ( uch1 << 8 ) | uch2 );
                     cklen -= 2;
                     filelen -= 2;
                     if ( cklen >= 3 )
                     {
                        unsigned char uch3;

                        uch1 = read_byte( fp );
                        uch2 = read_byte( fp );
                        uch3 = read_byte( fp );
                        printf( "  PC: %6X", ( uch1 << 16 ) |
                                ( uch2 << 8 ) | uch3 );
                        cklen -= 3;
                        filelen -= 3;
                     }
                  }
               }
               putchar( '\n' );
            }
         }
      }
      else if ( !strncmp( id, "CMem", 4 ) )
      {
         printf( " (compressed memory)" ); 
         if ( status & GOT_MEMORY )
         {
            printf( "\n*** warning: later memory chunk will be ignored by readers.\n" );
            ++warnings;
         }
         else
         {
#if defined SHORT_DUMP          
         uint32_t ul1;

         for ( ul1 = 0; ul1 < cklen; ++ul1 )
         {                      
            unsigned char uch = read_byte( fp ); 

            coredump( ul1, uch ); 
         }                      
         filelen -= cklen;
         cklen = 0;             
#elif defined LONG_DUMP         
         uint32_t ul1, ul2;

         for ( ul1 = 0, ul2 = 0; ul1 < cklen; ++ul1 )
         {                      
            unsigned char uch = read_byte( fp ); 

            if ( !uch )
            {                   
               int c = read_byte( fp ) + 1;

               ++ul1;           
               while ( c-- )    
                  coredump( ul2++, '\0' ); 
            }
            else                
               coredump( ul2++, uch ); 
         }                      
         putchar( '\n' );       
         filelen -= cklen;
         cklen = 0;             
#endif 
            status |= GOT_MEMORY | GOT_CMEM;
         }
      }
      else if ( !strncmp( id, "UMem", 4 ) )
      {
         printf( " (uncompressed memory)\n" );
         if ( status & GOT_MEMORY )
         {
            printf( "*** warning: later memory chunk will be ignored by readers.\n" );
            ++warnings;
         }
         else
            status |= GOT_MEMORY | GOT_UMEM;
      }
      else if ( !strncmp( id, "Stks", 4 ) )
      {
         printf( " (stacks)\n" );
         if ( status & GOT_STACKS )
         {
            printf( "*** warning: later Stks chunk will be ignored by readers.\n" );
            ++warnings;
         }
         else
         {
            if ( !check_stacks( fp, cklen ) )
               ++errors;
            filelen -= cklen;
            cklen = 0;
            status |= GOT_STACKS;
         }
      }
      else if ( !strncmp( id, "IntD", 4 ) )
      {
         printf( " (interpreter-dependent)" ); 
         if ( cklen >= 4 )      
         {                      
            unsigned char uch1; 

            fputs( "    Operating system ", stdout ); 
            for ( i = 0; i < 4; ++i ) 
            {                   
               putchar( read_byte( fp ) ); 
            }                   
            cklen -= 4;
            filelen -= 4;       
            if ( cklen >= 1 )   
            {                   
               fputs( "  flags ", stdout ); 
               uch1 = read_byte( fp ); 
               if ( uch1 & 0x02 ) 
               {                
                  putchar( 's' ); 
               }                
               if ( uch1 & 0x01 ) 
               {                
                  putchar( 'c' ); 
               }                
               cklen -= 1;
               filelen -= 1;    
               if ( cklen >= 1 ) 
               {                
                  printf( "  contents id %02X", read_byte( fp ) ); 
                  cklen -= 1;
                  filelen -= 1;
                  if ( cklen >= 6 ) 
                  {             
                     ( void ) read_byte( fp ); /* discard */
                     ( void ) read_byte( fp ); /* discard */
                     cklen -= 2;
                     filelen -= 2; 
                     fputs( "    Interpreter ", stdout ); 
                     for ( i = 0; i < 4; ++i ) 
                     {          
                        putchar( read_byte( fp ) ); 
                     }          
                     cklen -= 4;
                     filelen -= 4; 
                     printf( "  %" PRIu32 " bytes of data", cklen );
                  }             
               }                
            }                   
         }                      
         putchar( '\n' );       
      }
      else if ( !strncmp( id, "ANNO", 4 ) )
      {
         printf( " (annotation)" ); 
         if ( cklen >= 1 )      
         {                      
            uint32_t l;

            fputs( "    ", stdout ); 
            for ( l = 0; l < cklen; ++l ) 
            {                   
               putchar( read_byte( fp ) ); 
            }                   
            filelen -= cklen;
            cklen = 0;          
         }                      
         putchar( '\n' );       
      }
      else if ( !strncmp( id, "AUTH", 4 ) )
      {
         printf( " (author)" ); 
         if ( cklen >= 1 )      
         {                      
            uint32_t l;

            fputs( "    ", stdout ); 
            for ( l = 0; l < cklen; ++l ) 
            {                   
               putchar( read_byte( fp ) ); 
            }                   
            filelen -= cklen;
            cklen = 0;          
         }                      
         putchar( '\n' );       
      }
      else if ( !strncmp( id, "NAME", 4 ) )
      {
         printf( " (name of content)" ); 
         if ( cklen >= 1 )      
         {                      
            uint32_t l;

            fputs( "    ", stdout ); 
            for ( l = 0; l < cklen; ++l ) 
            {                   
               putchar( read_byte( fp ) ); 
            }                   
            filelen -= cklen;
            cklen = 0;          
         }                      
         putchar( '\n' );       
      }
      else if ( !strncmp( id, "(c) ", 4 ) )
      {
         printf( " (copyright on content)" ); 
         if ( cklen >= 1 )      
         {                      
            uint32_t l;

            fputs( "    ", stdout ); 
            for ( l = 0; l < cklen; ++l ) 
            {                   
               putchar( read_byte( fp ) ); 
            }                   
            filelen -= cklen;
            cklen = 0;          
         }                      
         putchar( '\n' );       
      }
      else if ( !strncmp( id, "    ", 4 ) )
      {
         printf( " (%" PRIu32 "-byte filler)\n", cklen );
      }
      else
      {
         printf( " (unknown chunk; skipped)\n" );
      }
      filelen -= 8;

      if ( cklen > filelen || filelen - cklen < odd_byte )
      {
         printf( "*** Chunk extends past end of IFZS chunk (%" PRIu32
               " left in IFZS)\n", filelen );
         exit( EXIT_FAILURE );
      }
      filelen -= cklen;
      for ( ; cklen > 0; --cklen )
      {
         ( void ) read_byte( fp ); /* skip chunk contents */
      }
      if ( odd_byte )
      {
         --filelen;
         ( void ) read_byte( fp ); /* skip pad byte */
      }
   }

   if ( getc( fp ) != EOF )
   {
      uint64_t trailing_bytes;

      for ( trailing_bytes = 1; getc( fp ) != EOF; ++trailing_bytes ) ;
      printf( "*** warning: spurious data (%" PRIu64
            " bytes) past specified FORM length\n",
            trailing_bytes );
      ++warnings;
   }

   if ( argc == 2 && strcmp( argv[1], "-" ) )
   {
      fclose( fp );
   }

   i = EXIT_SUCCESS;

   if ( ( status & GOT_ALL ) != GOT_ALL )
   {
      printf( "\n*** Missing chunks:" );
      if ( !( status & 0x01 ) )
         printf( " IFhd" );
      if ( !( status & 0x02 ) )
         printf( " CMem/UMem" );
      if ( !( status & 0x04 ) )
         printf( " Stks" );
      printf( "\n" );
      i = EXIT_FAILURE;
   }

   if ( errors > 0 )
   {
      printf( "\n*** %u error%s.\n", errors, ( errors > 1 ) ? "s" : "" );
      i = EXIT_FAILURE;
   }
   else
   {
      if ( warnings > 0 )
         printf( "\n*** %u warning%s.\n", warnings, ( warnings > 1 ) ? "s" : "" );
      printf( "\nSave file is valid.\n" );
   }

   return ( i );
}
