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
#include <unistd.h>

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

/* utility function prototypes */
void    start_shell( void );
void    parse_input( char* );
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

/* alias handling */
int     handle_aliases( void );
int     check_for_alias( void );

/* env variable handling */
int     handle_env_vars( void );
int     convert_env_var( int );

/* directory change handling */
char*   get_parent_dir( void );
int     handle_directory_change( void );
char*   set_new_dir( void );




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
    print_commands();

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

    puts( "processing commands..." );
    // suggested order of processing: 

    // handle all alias processing 
    handle_aliases();

    // check for environment variables that need to be translated
    handle_env_vars();

    // check for cd
    handle_directory_change(); 
        // will probably have to create a stack to track directories

    // DONE - check for aliases that need to be added
    // check for aliases that need to be removed
    // check for input/output redirection
    // check for pipes
    // check for background processes

    print_commands();
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
    if ( strcmp( cmds[0], "cd" ) != 0 && n_cmds < 2 )
        return FAILURE; 

    char* new_dir = NULL;

    if ( strcmp( cmds[1], ".." ) == 0 || 
         strcmp( cmds[1], "./.." ) == 0 
       )
    {
        if ( chdir( get_parent_dir() ) < 0 )
        {
            fprintf( stderr, "Error changing to parent directory. \
                Please provide a correct path.\n" );

            return FAILURE;
        } 

        if( ( setenv( "PWD", get_parent_dir(), 1 ) ) < 0 )
            puts( "Error. Could not change $PWD" );
    }
    else
    {
        if ( ( new_dir = set_new_dir() ) == NULL )
        {
            fprintf( stderr, "Error. Could not set new directory.\n" );
            return FAILURE;
        }            

        if( is_directory( new_dir ) == 0 )
        {
            fprintf( stderr, "Error. %s is not a directory.\n", 
                     new_dir );

            return FAILURE;
        }

        if ( chdir( new_dir ) < 0 )
        {
            fprintf( stderr, "Error changing directories. Please \
                provide a correct path.\n" );

            return FAILURE;
        }


        /* reset env variables */
        if( ( setenv( "PWD", new_dir, 1 ) ) < 0 )
            puts( "Error. Could not change $PWD" );

        free( new_dir );
        new_dir = NULL;

    }

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
char* get_parent_dir( void )
{
    char* path = getenv( "PWD" );
    int end_parent = strlen( path ) - 1;
    char *parent_path = NULL; 

    while( path[end_parent] != '/' )
        end_parent--;

    parent_path = (char*) malloc( ( end_parent + 1 ) * sizeof(char) );
    if ( parent_path == NULL )
    {
        fprintf( stderr, "Error allocating memory for parent path.\n" );
        return NULL;
    }

    strncpy( parent_path, path, end_parent );

    parent_path[end_parent] = N_TERM;

    return parent_path;
}


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
char* set_new_dir( )
{
    char* cur_dir = getenv( "PWD" );
    char* new_dir = NULL;

    /* if its a relative path */
    if( strncmp( cmds[1], "./", 2 ) == 0 )
    {
        /* create new pwd */
        new_dir = (char*) malloc( ( strlen(&cmds[1][2]) + 
                                    strlen( cur_dir ) +
                                    2 
                                  ) * sizeof(char)
                                );

        if ( new_dir == NULL )
        {
            fprintf( stderr, "Error allocating memory for setenv.\n" );
            return NULL;
        }

        strncpy( new_dir, cur_dir, strlen( cur_dir ) );
        new_dir[strlen( cur_dir )] = '/';
        new_dir[strlen( cur_dir ) + 1] = N_TERM;

        strcat( new_dir, &cmds[1][2] );
    }
    else if( strncmp( cmds[1], "..", 2 ) == 0 )
    {
        /* create new pwd */
        new_dir = (char*) malloc( ( strlen(&cmds[1][2]) + 
                                    strlen( get_parent_dir() ) +
                                    1 
                                  ) * sizeof(char)
                                );

        if ( new_dir == NULL )
        {
            fprintf( stderr, "Error allocating memory for setenv.\n" );
            return NULL;
        }

        strncpy( new_dir, get_parent_dir(), 
                 strlen( get_parent_dir() ) );

        new_dir[strlen( get_parent_dir() )] = N_TERM;

        strcat( new_dir, &cmds[1][2] );
    }
    else if( strncmp( cmds[1], "/", 1 ) != 0 )
    {
        /* case it is a relative path from current directory */
        new_dir = (char*) malloc( ( strlen( cur_dir ) + 
                                    strlen( cmds[1] ) +
                                    2 
                                  ) * sizeof(char)
                                );

        if ( new_dir == NULL )
        {
            fprintf( stderr, "Error allocating memory for setenv.\n" );
            return NULL;
        }

        strncpy( new_dir, cur_dir, strlen( cur_dir ) );
        new_dir[strlen( cur_dir )] = '/';
        new_dir[strlen( cur_dir ) + 1] = N_TERM;

        strcat( new_dir, cmds[1] );
    }
    else
    {
        new_dir = (char*) malloc( ( strlen( cmds[1] ) + 1 ) 
                                           * sizeof(char) );

        if ( new_dir == NULL )
        {
            fprintf( stderr, "Error allocating memory for setenv.\n" );
            return NULL;
        }

        strcpy( new_dir, cmds[1] );
    }

    return new_dir;
} /* end set_new_dir() */



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
}
