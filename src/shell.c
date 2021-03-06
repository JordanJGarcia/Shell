/*********************************************************************/
/*                                                                   */
/*      Author: Jordan Garcia                                        */
/*      Date:   12/09/2019                                           */
/*      Project:                                                     */
/*                                                                   */
/*          This project is my implementation of a simple shell.     */
/*                                                                   */
/*********************************************************************/

// BUGS:
    // Cannot do redirection with echo...

// NEXT STEPS:
    // 1) Revise documentation to make sure every thing is accurate
    // 2) figure out why shell sometimes takes 2 exits
    // 3) fix any ineffecient areas of the system
    // 4) add ability to read in permanent aliases from .j_profile
    // 5) add any other features you think of! :D
    // 6) make sure memory leaks don't exist


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/* for custom libraries */
#include "../lib/alias.h"
#include "../lib/string_module.h"
#include "../lib/command_history.h"
#include "../lib/execution.h"

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
int     n_pipes = 0; 
char    current_path[PROMPT_SIZE];

/* utility function prototypes */
void    start_shell( void );
void    parse_input( char* );
int     process_commands( void );

/* helper function (low level) */
int     is_directory( const char* );
int     is_reg_file( const char* );
void    print_commands( void );

/* history handling */
int     handle_history( void );

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
int     process_path( void );

/* program execution function prototypes */
int     handle_program_execution( void );
int     is_redirection( void );
int     is_pipe( void );



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

    /* begin infinite loop to control shell */
    while ( 1 )
    {
        /* prompt then read line - line is allocated with malloc(3) */
        line = readline(prompt);

        /* check if user wants to exit the shell */
        if ( strcmp( line, "exit" ) == 0 )
        {
            puts( "Now exiting the best shell ever created... :(\n" );
            free( line );
            free_history();
            free_aliases();
            return;
        }
        else
            parse_string( line, &cmds, &n_cmds, &n_pipes );

        //print_commands();


        if ( cmds != NULL )
            if ( process_commands() == FAILURE )
                ;

        /* free all memory */
        free( line );

        for ( int i = 0; i < n_cmds; i++ )
            free( cmds[i] );

        free( cmds );

        n_cmds = 0;
        n_pipes = 0;
    }

    free_history();
    free_aliases();
    return;
}/* end start_shell() */


/*********************************************************************/
/*                                                                   */
/*      Function name: process_commands                              */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Handles parsed commands for appropriate processing.      */
/*          After processing certain commands, there is no need to   */
/*          process the rest. I.E. printing history means we can     */
/*          print the command history and continue without worrying  */
/*          about handling other events in this loop iteration.      */
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

    // handle printing of history
    if( handle_history() == SUCCESS )
    {
        add_cmds_to_history( cmds, n_cmds );
        return SUCCESS; 
    }

    // handle all alias processing 
    if ( handle_aliases() == FAILURE )
    {
        add_cmds_to_history( cmds, n_cmds );
        return FAILURE;
    }

    // handle environmental variable translations
    handle_env_vars();

    // handle directory changes
    if( handle_directory_change() == SUCCESS )
    {
        add_cmds_to_history( cmds, n_cmds );
        return SUCCESS; 
    }
        
    // handle program execution
    handle_program_execution();
        // check for input/output redirection
        // check for pipes
        // check for background processes

    /* add to history */
    add_cmds_to_history( cmds, n_cmds );

    //print_commands();
    return SUCCESS;
}/* end process_commands */


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_history                                */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          prints history of commands entered.                      */
/*                                                                   */
/*********************************************************************/
int handle_history( void )
{
    if ( strcmp( cmds[0], "history" ) == 0 )
    {
        print_history( stdout ); 
        return SUCCESS;
    }
    return FAILURE;
}


/*********************************************************************/
/*                                                                   */
/*      Function name: check_for_alias                               */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Checks for aliases and converts them if found.           */
/*          Return failure anytime an alias isnt being translated.   */
/*          This is done so that way we know when we dont need to    */
/*          use the alias any further.                               */
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
        return FAILURE;
    }
    else if ( strcmp( cmds[0], "unalias" ) == 0 )
    {
        if( n_cmds < 2 )
        {
            fprintf( stderr, "Error, no alias specified to remove.\n" );
            return FAILURE;

        }
        remove_alias( cmds[1] );
        return FAILURE;
    }
    else if ( n_cmds == 2 && 
              strcmp( cmds[0], "show" ) == 0 && 
              strcmp( cmds[1], "aliases" ) == 0 
            )
    {
        print_aliases();
        return FAILURE;
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
            {
                printf("Error: Cannot switch to HOME directory.\n" );
                return FAILURE;
            }
            else
            {
                setenv( PWD, getenv( "HOME" ), 1 );
                return SUCCESS; 
            }
        }
    }

    /* switching to any other directory */
    if ( chdir( cmds[1] ) != 0 )
    {
        printf( "Error: Cannot change directory to %s\n", cmds[1] );
        return FAILURE;
    }

    return SUCCESS;
} /* end handle_directory_change() */


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
    int r_flag = is_redirection(); 
    int p_flag = is_pipe();

    switch ( r_flag )
    {
        case INPUT:
            // input redirection
            if ( p_flag == SUCCESS )
                ;//redirect_input_and_pipe();

            redirect_input();
            break;
        case OUTPUT:
            // output redirection
            if ( p_flag == SUCCESS )
                ;//redirect_output_and_pipe();

            redirect_output(); 
            break;
        case INOUT:
            // input & output redirection
            if ( p_flag == SUCCESS )
                ;//redirect_both_and_pipe();

            redirect_input_and_output();
            break;
        default:
            // no redirection
            if ( p_flag == SUCCESS )
                execute_and_pipe( n_pipes ); 
            else
                execute();

            break; 
    }
    return SUCCESS;
}




/* LOW LEVEL FUNCTIONS START POINT */




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
/*      Function name: is_redirection                                */
/*      Return type:   int                                           */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          returns a value depending on type of redirection.        */
/*              0: no redirection                                    */
/*              1: output redirection                                */
/*             -1: input redirection                                 */
/*              2: both input and output redirection.                */
/*                                                                   */
/*********************************************************************/
int is_redirection( void )
{
    int ctr = 0;
    int o_flag = FAILURE, i_flag = FAILURE;

    for ( ; ctr < n_cmds; ctr++ )
    {
        if ( strcmp( cmds[ctr], "<" ) == 0 )
            i_flag = SUCCESS;

        if ( strcmp( cmds[ctr], ">" ) == 0 )
            o_flag = SUCCESS; 
    }

    if ( o_flag == SUCCESS && i_flag == SUCCESS )
        return INOUT;
    else if ( o_flag == SUCCESS ) 
        return OUTPUT;
    else if ( i_flag == SUCCESS )
        return INPUT;

    return FAILURE; 
}


/*********************************************************************/
/*                                                                   */
/*      Function name: is_pipe                                       */
/*      Return type:   int                                           */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          returns a value depending on type of redirection.        */
/*              0: no pipe                                           */
/*              3: pipe                                              */
/*********************************************************************/
int is_pipe( void )
{
    return ( n_pipes == 0 ? FAILURE : SUCCESS );
}


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
