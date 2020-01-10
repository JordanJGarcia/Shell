/*********************************************************************/
/*                                                                   */
/*      Author: Jordan Garcia                                        */
/*      Date:   12/09/2019                                           */
/*      Project:                                                     */
/*                                                                   */
/*          This project is my implementation of a simple shell.     */
/*                                                                   */
/*********************************************************************/


// NEED TO FIGURE OUT HOW TO INCORPORATE PROCESS_QUOTED_STRING() INTO PROGRAM
// FIGURE OUT HOW TO INCORPORATE BSEARCH INTO ALIASES.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

/* for custom libraries */
#include "../lib/alias.h"
#include "../lib/string_module.h"

/* macros */
#define PROMPT_SIZE 255
#define N_TERM '\0'
#define FAILURE 0
#define SUCCESS 1
#define QUOTE_MARKER "**"
#define PWD "PWD"
#define USER "USER"
#define HOST "HOST"

/* global variables */
//char*   cmd = NULL; 
char**  cmds = NULL; 
int     n_cmds = 0; 
char    current_path[PROMPT_SIZE];

/* utility function prototypes */
void    start_shell( void );
void    parse_input( char* );
int     process_commands( void );
int     process_quoted_string( char*, int );

/* helper function (low level) */
int     is_directory( const char* );
int     is_reg_file( const char* );
void    print_commands( void );

/* alias handling */
int     handle_aliases( void );
int     check_for_alias( void );

/* env variable handling */
int     handle_env_vars( void );
int     convert_env_var( int );

/* directory change handling */
int     handle_directory_change( void );
char*   get_parent_dir( int );
int     set_new_dir( void );
int     count_parents( const char* );
char*   get_last_parent( const char* str );

/* echo handling */
int     handle_echo( void );

/* program execution function prototypes */
int     handle_program_execution( void );
void    execute( void );
void    execute_redirection( char ); /* char signals I|O redirection */
void    execute_multiple_redirection( void );
void    execute_pipe( void );


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
    if( getenv(HOST) == NULL )
        sprintf( prompt, "%s@UnknownHost> ", getenv(USER) );
    else
        sprintf( prompt, "%s@%s> ", getenv(USER), getenv(HOST) );

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
            parse_string( line, &cmds, &n_cmds );

        print_commands();


        if ( cmds != NULL )
            if ( process_commands() == FAILURE )
                ;

        free( line );

        for ( int i = 0; i < n_cmds; i++ )
            free( cmds[i] );

        free( cmds );

        n_cmds = 0;
    }

    return;
}/* end start_shell() */


int process_quoted_string( char* str, int index )
{
    char** split_string = NULL;
    int n_strings = 0; 

    // go through string and parse it
    parse_string( str, &split_string, &n_strings );

    // move_strings_down by number of commands in alias
    move_strings_down( &cmds, &n_cmds, n_strings, index );

    // add_strings() to cmds 
    add_strings( &cmds, &split_string, index, n_strings );

    puts( "\nafter processing quoted string: " );
    print_commands(); 

    return SUCCESS;
}


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
    if ( handle_aliases() == FAILURE )
        return FAILURE;

    // handle environmental variable translations
    if ( handle_env_vars() == FAILURE )
        ; //return FAILURE;

    // handle directory changes
    handle_directory_change(); 

    // handle echo command 
    handle_echo(); 
        
    // handle program execution
    handle_program_execution();


    // check for input/output redirection
    // check for pipes
    // check for background processes

    //print_commands();
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
int handle_aliases( void )
{
    if ( strcmp( cmds[0], "alias" ) == 0 )
    {
        if( n_cmds < 4 )
        {
            fprintf( stderr, "Error, no alias specified to add.\n" );
            return FAILURE;
        }
        add_alias( cmds[1], cmds[3] );
    }
    else if ( strcmp( cmds[0], "unalias" ) == 0 )
    {
        if( n_cmds < 2 )
        {
            fprintf( stderr, "Error, no alias specified to remove.\n" );
            return FAILURE;

        }
        remove_alias( cmds[1] );
    }
    else if ( n_cmds == 2 && 
              strcmp( cmds[0], "show" ) == 0 && 
              strcmp( cmds[1], "aliases" ) == 0 
            )
    {
        print_aliases();
    }
    else
        check_for_alias();    

    return SUCCESS; 
}


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_env_vars                               */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          checks for environmental variables and calls conversion  */
/*          function.                                                */
/*                                                                   */
/*********************************************************************/
int handle_env_vars( void )
{
    int counter = 0; 

    for( ; counter < n_cmds; counter++ )
    {
        /* if we find a possible environmental variable */
        if( strcmp( cmds[counter], "$" ) == 0 )
            if ( convert_env_var( counter ) == SUCCESS )
                counter--; 
    }
    return SUCCESS; 
}

// handle punctuation issues here. 


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_directory_change                       */
/*      Return type:   int.                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          determines and conducts directory change if needed.      */
/*                                                                   */
/*********************************************************************/
int handle_directory_change( void )
{
    char* new_dir = NULL;

    /* ensure we want to switch directories */
    if ( strcmp( cmds[0], "cd" ) != 0 )
        return FAILURE;

    /* switching to home directory */
    if ( n_cmds == 1 || 
         strcmp( cmds[1], "~/" ) == 0 || 
         strcmp( cmds[1], "~" ) == 0 
       )
    {
        if( is_directory( getenv( "HOME" ) ) != 0 )
        {
            if( chdir( getenv( "HOME" ) ) != 0 )
                return FAILURE;
            else
            {
                setenv( PWD, getenv( "HOME" ), 1 );
                return SUCCESS; 
            }
        }
    }

    /* absolute path directory */
    if ( cmds[1][0] == '/' )
    {
        if ( is_directory( cmds[1] ) != 0 )
        {
            if ( chdir( cmds[1] ) != 0 )
                return FAILURE;
            else
            {
                setenv( PWD, cmds[1], 1 );
                return SUCCESS;
            }
        }
    }

    /* relative path directory */ 
    set_new_dir();

    if ( is_directory( current_path ) != 0 )
    {
        if ( chdir( current_path ) != 0 )
            return FAILURE;
        else
            setenv( PWD, current_path, 1 );
    }

    return SUCCESS;
}


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_echo                                   */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          mimics shell echo utility. Does not translate            */
/*          environmental variables inside quotes. :(                */
/*                                                                   */
/*********************************************************************/
int handle_echo( void ) 
{
    if ( strcmp( cmds[0], "echo" ) != 0 )
        return FAILURE; 

    int ctr = 1;   

    for ( ; ctr < n_cmds; ctr++ )
    {
        if ( strncmp( cmds[ctr], QUOTE_MARKER, 2 ) == 0 &&
             strlen( cmds[ctr] ) > 2 
           )
        {
            process_quoted_string( cmds[ctr], ctr );
            handle_env_vars(); 

            /* disregard the quote signals (**) */
            printf( "%s ", &cmds[ctr][2] );
        }
        else
            printf( "%s ", cmds[ctr] );
    }

    puts( " " );
    return SUCCESS;  
}


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_program_execution                      */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Routes all program execution to their respected          */
/*          functions.                                               */
/*                                                                   */
/*********************************************************************/
int handle_program_execution( void )
{
    puts( "\n" );
    print_commands();
    return SUCCESS;
}


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

    for( int i = 0; i < n_cmds; i++ )
    {
        if( ( a_ptr = find_alias( cmds[i] ) ) != NULL )
        {
            /* create space for aliases */
            move_strings_down( &cmds, &n_cmds, a_ptr->n_cmds, i );

            /* replace alias in cmds */
            add_strings( &cmds, &(a_ptr->translated), i, a_ptr->n_cmds );

            return SUCCESS; 
        }
    }
    return FAILURE; 
}


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_env_vars                               */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          checks for environmental variables and calls conversion  */
/*          function.                                                */
/*                                                                   */
/*********************************************************************/
int convert_env_var( int index )
{
    char* env_var = getenv( cmds[index + 1] );

    if( env_var == NULL )
    {
        puts( "Could not translate environmental variable." );
        return FAILURE; 
    }

    /* translate env variable in commands */
    free( cmds[index + 1] );
    cmds[index + 1] = NULL;
    cmds[index + 1] = (char*) malloc( ( strlen( env_var ) + 1 ) 
                                        * sizeof(char) );

    if ( cmds[index + 1] == NULL )
    {
        fprintf(stderr, "Could not alocate memory for env variable.\n" );
        return FAILURE;
    }
    strcpy( cmds[index + 1], env_var );


    /* move all commands down one, removing '$' */
    for( ; index < n_cmds - 1; index++ )
    {
        free( cmds[index] );

        if( (cmds[index] = (char*) malloc( ( strlen( cmds[index + 1] )
                                             + 1 ) * sizeof(char) ) ) 
                                             == NULL
          )
        {
            fprintf( stderr, "Could not allocate memory to move \
                              commands for env variable.\n" );
        }

        strcpy( cmds[index], cmds[index +1] );
    }

    /* free last element and make null/ lower n_cmds */
    free( cmds[index] );
    cmds[index] = NULL;
    n_cmds -= 1;

    return SUCCESS;
}


/*********************************************************************/
/*                                                                   */
/*      Function name: get_parent_dir                                */
/*      Return type:   char*                                         */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          returns path to parent directory from current directory. */
/*                                                                   */
/*********************************************************************/
char* get_parent_dir( int n_par )
{
    char* path = getenv( PWD );
    int end_parent = strlen( path ) - 1;
    int num_par = 0;
    char *parent_path = NULL;

    for ( ; end_parent != 0; end_parent-- )
    {
        if ( num_par == n_par )
            break;

        if ( path[end_parent] == '/' )
            num_par++; 
    }

    parent_path = (char*) malloc( ( end_parent + 2 ) * sizeof(char) );
    
    if ( parent_path == NULL )
    {
        fprintf( stderr, "Error allocating memory for parent path.\n" );
        return NULL;
    }

    strncpy( parent_path, path, end_parent + 1 );
    parent_path[end_parent + 1] = N_TERM;

    return parent_path;
} /* end get_parent_dir() */


/*********************************************************************/
/*                                                                   */
/*      Function name: set_new_dir                                   */
/*      Return type:   char*                                         */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          creates absolute path for new directory.                 */
/*                                                                   */
/*********************************************************************/

// this funtion sometimes appends too many slashes, but it doesnt affect it. 
// maybe create a separate function to assign current_path where I test for that stuff
int set_new_dir( void )
{
    char* parent_dir = NULL;
    int num_parents;

    /* check if changing to home directory */
    if ( cmds[1][0] == '~' )
        sprintf( current_path, "%s/", getenv( "HOME" ) );
    else
        sprintf( current_path, "%s/", getenv( "PWD" ) );

    /* 
       check for a name after to switch to 
       another user's home directory 
    */

    /* append remaining path provided in command line */
    if( strncmp( cmds[1], "~/", 2 ) == 0 ||
        strncmp( cmds[1], "./", 2 ) == 0
      )
        strcat( current_path, &cmds[1][2] );
    else
        strcat( current_path, cmds[1] );

    /* see if any parent directories (check for existence of '../') */
    num_parents = count_parents( current_path );

    /* if there are no parents to convert, return path */
    if ( num_parents == 0 )
        return SUCCESS;

    /* get the index of the last parent within the string */
    char* loc = get_last_parent( current_path );
    long last_par_index = loc - current_path; 

    /* get parent dir absolute path */
    parent_dir = get_parent_dir( num_parents );

    /* if no directories to append after parents */
    if ( strlen( current_path ) - 2 == last_par_index )
    {
        sprintf( current_path, "%s/", parent_dir );
        free( parent_dir );
        return SUCCESS;
    }

    /* append any remaining directories after parents */
    sprintf( current_path, "%s/%s", parent_dir, loc + 2 );

    return SUCCESS;
} /* end set_new_dir() */



/*********************************************************************/
/*                                                                   */
/*      Function name: count_parents                                 */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char* str: string to search.                             */
/*                                                                   */
/*      Description:                                                 */
/*          calculates number of parents directories.                */
/*                                                                   */
/*********************************************************************/
int count_parents( const char* str )
{
    char* loc = strstr( str, ".." );
    if( loc == NULL )
        return 0;

    return 1 + count_parents( loc + 2 );
} /* end count_parents */


/*********************************************************************/
/*                                                                   */
/*      Function name: get_last_parent                               */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char* str: string to search.                             */
/*                                                                   */
/*      Description:                                                 */
/*          returns index of last parent (..) dir in str - 2         */
/*                                                                   */
/*********************************************************************/
char* get_last_parent( const char* str )
{
    char* loc = strstr( str, ".." );
    char* dummy = loc;

    while( dummy != NULL )
    {
        loc = dummy; 
        dummy = get_last_parent( loc + 2 );
    }

    return loc; 
} /* end get_last_parent() */




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
} /* end is_reg_file() */
