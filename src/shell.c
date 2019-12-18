/*********************************************************************/
/*                                                                   */
/*      Author: Jordan Garcia                                        */
/*      Date:   12/09/2019                                           */
/*      Project:                                                     */
/*                                                                   */
/*          This project is my implementation of a simple shell.     */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* global variables */
char* cmd; 
char** cmds; 
int n_cmds = 0; 

/* macros */
#define PROMPT_SIZE 255
#define N_TERM '\0'

/* utility function prototypes */
void    parse_input( char* );
void    add_command( void );
void    build_command( char ); 
void    start_shell( void );
void    process_commands( void );
void    print_commands( void );

/* helper function (low level) */
int     is_directory( const char* );
int     file_exists( const char* );

/* command processing function prototypes */
void    execute( void );
void    execute_redirection( char ); /* char determines if its I/O redirection */
void    execute_multiple_redirection( void );
void    execute_pipe( void );





/*********************************************************************/
/*                                                                   */
/*      Function name: main()                                        */
/*      Return type:   int                                           */
/*      Parameter(s):  None                                          */
/*      Description:                                                 */
/*          main() will start the shell prompt.                      */
/*                                                                   */
/*********************************************************************/
int main( void )
{
    start_shell();
    return EXIT_SUCCESS;
} /* end main */


/*********************************************************************/
/*                                                                   */
/*      Function name: start_shell                                   */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*      Description:                                                 */
/*          start_shell() will begin a loop prompting the user       */
/*          for commands and call functions to tokenize and process  */
/*          those commands.                                          */
/*                                                                   */
/*********************************************************************/
void start_shell( void )
{
    char prompt[PROMPT_SIZE];
    
    /* assign prompt for readline */
    if( getenv("HOST") == NULL )
        sprintf( prompt, "%s@UnknownHost> ", getenv("USER") );
    else
        sprintf( prompt, "%s@%s> ", getenv("USER"), getenv("HOST") );

    char* line = NULL;

    while ( 1 )
    {
        /* prompt then read line - line is allocated with malloc(3) */
        line = readline(prompt);

        /* check if user wants to exit the shell */
        if( strcmp( line, "exit" ) == 0 )
            return;
        else
            parse_input( line );


        if( cmds != NULL )
            //process_commands( commands );

        free( line );

        for( int i = 0; i < n_cmds; i++ )
            free( cmds[i] );

        free( cmds );

        n_cmds = 0;
    }

    return;
}/* end start_shell() */


/*********************************************************************/
/*                                                                   */
/*      Function name: split_input                                   */
/*      Return type:   char**                                        */
/*      Parameter(s):  1                                             */
/*          char* line: line of commands user types in               */
/*                                                                   */
/*      Description:                                                 */
/*          This function splits the commands typed into an array    */
/*          of strings.                                              */
/*                                                                   */
/*********************************************************************/
void parse_input( char * line )
{
    int line_size = strlen( line );

    for( int i = 0; i < line_size; i++ )
    {
        /* special characters to watch out for */
        if ( line[i] == '$' || line[i] == '|' || 
             line[i] == '<' || line[i] == '>' || 
             line[i] == '&' 
           )
        {
            //puts( "Found special character!" );
            if( cmd != NULL )
                add_command();

            build_command( line[i] );
            add_command();
        }
        else if ( i == line_size - 1 ) /* end of line */
        {
            if( !isspace( line[i] ) )
                build_command( line[i] );

            add_command();
        }
        else if ( isspace( line[i] ) ) /* spacing */
            add_command();
        else /* everything else */
            build_command( line[i] ); 

        //free( command );
    }

    /* print out commands */
    print_commands();

    return;
} /* end split_input() */


/*********************************************************************/
/*                                                                   */
/*      Function name: add_command                                   */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*********************************************************************/
void add_command( void )
{
    /* if there hasn't been a command initialized */
    if( cmd == NULL )
        return;

    /* allocate memory to add new string to array */
    if( n_cmds == 0 )
        cmds = (char**)malloc( sizeof(char*) );
    else
        cmds = (char**)realloc( cmds, (n_cmds + 1) 
                                    * sizeof(char*) );

    /* allocate memory for the actual string in the array */
    cmds[n_cmds] = (char*)malloc( strlen(cmd) + 1 );
    strcpy( cmds[n_cmds], cmd );

    /* increase command count */
    n_cmds += 1;

    /* allocate memory to add null term */
    cmds = (char**)realloc( cmds, (n_cmds + 1) * 
                                sizeof(char*) );

    /* last element set to NULL for execv() to work */
    cmds[n_cmds] = NULL;

    /* free up memory */
    free( cmd );
    cmd = NULL;

    return;
}/* end add_command() */


/*********************************************************************/
/*                                                                   */
/*      Function name: build_command                                 */
/*      Return type:   char*                                         */
/*      Parameter(s):  1                                             */
/*          char c: char to add to command                           */
/*                                                                   */
/*      Description:                                                 */
/*          build_command accepts a character and appends it to the  */
/*          string "cmd." It creates a string one letter at a time.  */
/*                                                                   */
/*********************************************************************/
void build_command( char c )
{
    int size = ( cmd == NULL ? 0 : strlen(cmd) );

    if( size > 0 )
        cmd = (char*)realloc( cmd, (size + 2) * sizeof(char) );
    else
        cmd = (char*)malloc( (size + 2) * sizeof(char) );

    cmd[size] = c;
    cmd[size + 1] = N_TERM;
 
    return;
}/* end build_command() */


/*********************************************************************/
/*                                                                   */
/*      Function name: process_commands                              */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Accepts parsed commands for appropriate processing       */
/*                                                                   */
/*********************************************************************/
void process_commands( void )
{
    /* error checking */
    if ( n_cmds == 0 )
    {
        fprintf( stderr, "No commands to process.\n" );
        return;
    }

    // suggested order of processing: 

    // check for aliases that need to be translated
    // check for environment variables that need to be translated
    // check for cd
    // check for aliases that need to be added
    // check for aliases that need to be removed
    // check for input/output redirection
    // check for pipes
    // check for background processes

    return;
}/* end process_commands */


/*********************************************************************/
/*                                                                   */
/*      Function name: print_commands                                */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Prints the array of strings containing the parsed        */
/*          command line.                                            */
/*                                                                   */
/*********************************************************************/
void print_commands( void )
{
    puts("Printing commands...");
    for ( int i = 0; i < n_cmds; i++ )
        printf( "command %d: %s\n", i, cmds[i] );

    return;
}/* end print_commands() */


/*********************************************************************/
/*                                                                   */
/*      Function name: is_directory                                  */
/*      Return type:   int                                           */
/*      Parameter(s):  1                                             */
/*              const char*: directory name                          */
/*                                                                   */
/*      Description:                                                 */
/*          determines if filename is a directory                    */
/*                                                                   */
/*********************************************************************/
int is_directory( const char* filename )
{
    struct stat buffer;
    if( stat( filename, &buffer ) != 0 )
        return 0;
    
    return S_ISDIR( buffer.st_mode );
}/* end is_directory() */


/*********************************************************************/
/*                                                                   */
/*      Function name: file_exists                                   */
/*      Return type:   int                                           */
/*      Parameter(s):  1                                             */
/*              const char*: file name                               */
/*                                                                   */
/*      Description:                                                 */
/*          determines if file exists                                */
/*                                                                   */
/*********************************************************************/
int file_exists( const char* fileName )
{
    struct stat buffer;
    if( stat( fileName, &buffer ) != 0 )
        return 0;

    return S_ISREG( buffer.st_mode );
}
