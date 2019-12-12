/*********************************************************************/
/*                                                                   */
/*      Author: Jordan Garcia                                        */
/*      Date:   12/09/2019                                           */
/*      Project:                                                     */
/*                                                                   */
/*          This project is my implementation of a shell.            */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

/* global variables */
char* cmd; 
char** cmds; 
int n_cmds = 0; 

/* macros */
#define PROMPT_SIZE 255
#define N_TERM '\0'

/* function prototypes */
void    split_input( char* );
void    add_command( void );
void    build_command( char ); 
void    start_shell( void );
void    process_commands( char** );
void    print_commands( char** );


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
            split_input( line );


        if( cmds != NULL )
            //process_commands( commands );

        free( line );
        for( int i = 0; i < n_cmds; i++ )
            free( cmds[i] );

        free( cmds );
        n_cmds = 0;
    }
    //free( prompt );

    return;
}


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
void split_input( char * line )
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
            puts( "Found special character!" );
            //build_command( line[i] );
            //add_command();
        }
        else if ( i == line_size - 1 ) /* end of line */
        {
            if( !isspace( line[i] ) )
                build_command( line[i] );

            add_command();
            free( cmd );
            cmd = NULL;
        }
        else if ( isspace( line[i] ) ) /* spacing */
        {
            add_command();
            free( cmd );
            cmd = NULL;
        }
        else /* everything else */
        {
            build_command( line[i] ); 
            //printf( "Full command: %s\n", cmd );
        }

        //free( command );
    }

    /* print out commands */
    print_commands( cmds );

    return;
}

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
    else
        printf( "adding command: %s\n", cmd );

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

    return;
}


/*********************************************************************/
/*                                                                   */
/*      Function name: build_command                                 */
/*      Return type:   char*                                         */
/*      Parameter(s):  1                                             */
/*                                                                   */
/*********************************************************************/
void build_command( char c )
{
    int size = ( cmd == NULL ? 0 : strlen(cmd) );

    if( size > 0 )
        cmd = realloc( cmd, (size + 2) * sizeof(char) );
    else
        cmd = malloc( (size + 2) * sizeof(char) );

    cmd[size] = c;
    cmd[size + 1] = N_TERM;
 
    return;
}


/*********************************************************************/
/*                                                                   */
/*      Function name: print_commands                                */
/*      Return type:   void                                          */
/*      Parameter(s):  1                                             */
/*                                                                   */
/*********************************************************************/
void print_commands( char** commands )
{
    puts("Printing commands...");
    for ( int i = 0; i < n_cmds; i++ )
        printf( "command %d: %s\n", i, cmds[i] );
}

