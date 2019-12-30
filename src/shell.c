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

/* for custom libraries */
#include "../lib/alias.h"
#include "../lib/string_module.h"

/* global variables */
char*   cmd = NULL; 
char**  cmds = NULL; 
int     n_cmds = 0; 

/* macros */
#define PROMPT_SIZE 255
#define N_TERM '\0'
#define FAILURE 0
#define SUCCESS 1
#define T 1
#define F 0

/* utility function prototypes */
void    start_shell( void );
void    parse_input( char* );
//void    add_command( void );
int     process_commands( void );
void    print_commands( void );

/* helper function (low level) */
int     is_directory( const char* );
int     is_reg_file( const char* );

/* command processing function prototypes */
void    execute( void );
void    execute_redirection( char ); /* char signals I|O redirection */
void    execute_multiple_redirection( void );
void    execute_pipe( void );
void    echo_commands( void );
int     check_for_alias( void );



/*********************************************************************/
/*                                                                   */
/*      Function name: main()                                        */
/*      Return type:   int                                           */
/*      Parameter(s):  None                                          */
/*      Description:                                                 */
/*          main() will start the shell.                             */
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
        if ( strcmp( line, "exit" ) == 0 )
        {
            puts( "Now exiting the best shell ever created... :(\n" );
            return;
        }
        else
            parse_input( line );


        if ( cmds != NULL )
            if ( process_commands() == FAILURE )
            {
                fprintf( stderr, "Error processing commands. Continuing program \
                        execution. Type \"exit\" to leave shell.\n" );
            }

        free( line );

        for ( int i = 0; i < n_cmds; i++ )
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
                add_string( &cmd, &cmds, &n_cmds );

            build_string( line[i], &cmd );
            add_string( &cmd, &cmds, &n_cmds );
        }
        else if ( i == line_size - 1 ) /* end of line */
        {
            if( !isspace( line[i] ) )
                build_string( line[i], &cmd );

            add_string( &cmd, &cmds, &n_cmds );
        }
        else if( line[i] == '=' )
        {
            if( cmd != NULL )
                add_string( &cmd, &cmds, &n_cmds );

            build_string( line[i], &cmd );
            add_string( &cmd, &cmds, &n_cmds );
        }
        else if ( line[i] == '\"' || line[i] == '\'' ) /* string in quotes */
        {
            char term = line[i];
            i++;

            // build command with everything inside quotes
            do
            {
                /* if user forgot end quote, continue */
                if( i == line_size - 1 )
                {
                    build_string( line[i], &cmd );
                    break;
                }

                build_string( line[i++], &cmd );
            }while( line[i] != term );

            add_string( &cmd, &cmds, &n_cmds );
        }
        else if ( isspace( line[i] ) ) /* spacing */
            add_string( &cmd, &cmds, &n_cmds );
        else /* everything else */
            build_string( line[i], &cmd ); 

        //free( command );
    }

    /* print out commands */
    //print_commands();

    return;
} /* end split_input() */


/*********************************************************************/
/*                                                                   */
/*      Function name: process_commands                              */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Accepts parsed commands for appropriate processing       */
/*                                                                   */
/*********************************************************************/
int process_commands( void )
{
    /* error checking */
    if ( n_cmds == 0 )
    {
        fprintf( stderr, "No commands to process.\n" );
        return FAILURE;
    }

    // suggested order of processing: 

    // handle all alias processing 
    if ( strcmp( cmds[0], "alias" ) == 0 )
        add_alias( cmds[1], cmds[3] );
    else if ( strcmp( cmds[0], "unalias" ) == 0 )
        remove_alias( cmds[1] );
    else if ( strcmp( cmds[0], "show" ) == 0 && 
        strcmp( cmds[1], "aliases" ) == 0 )
    {
        print_aliases();
    }
    else
        check_for_alias();

    //print_commands();
    // check for environment variables that need to be translated
    // check for cd
        // will probably have to create a stack to track directories

    // DONE - check for aliases that need to be added
    // check for aliases that need to be removed
    // check for input/output redirection
    // check for pipes
    // check for background processes

    return SUCCESS;
}/* end process_commands */


/*********************************************************************/
/*                                                                   */
/*      Function name: check_for_alias                               */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          checks for aliases and converts them if found            */
/*                                                                   */
/*********************************************************************/
int check_for_alias( void ) 
{
    alias* a_ptr;
    char result = 'U'; //undetermined 

    for( int i = 0; i < n_cmds; i++ )
    {
        if( ( a_ptr = find_alias( cmds[i] ) ) != NULL )
        {
            puts( "Found an alias!" );

            move_strings_down( &cmds, &n_cmds, a_ptr->n_cmds, i );
            puts( "Adjusted string to add alias." );
            print_commands();

            /* replace alias in cmds */
            add_strings( &cmds, &(a_ptr->translated), i, a_ptr->n_cmds, T );
            puts( "Commands with replaced alias" );
            print_commands();

            return SUCCESS; 
        }
    }

    return FAILURE; 
}


/*********************************************************************/
/*                                                                   */
/*      Function name: echo_commands                                 */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          mimics shell echo utility                                */
/*                                                                   */
/*********************************************************************/
void echo_commands( void ) 
{
    return; 
}



/* LOW LEVEL FUNCTIONS START POINT */





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
    for ( int i = 0; i <= n_cmds; i++ )
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
int is_reg_file( const char* fileName )
{
    struct stat buffer;
    if( stat( fileName, &buffer ) != 0 )
        return 0;

    return S_ISREG( buffer.st_mode );
}
