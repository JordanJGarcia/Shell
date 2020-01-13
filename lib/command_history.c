#include "command_history.h"

/* globals */
int history_count = 0;

/*********************************************************************/
/*                                                                   */
/*      Function name: add_cmds_to_history                           */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char** cmds:   command to add to history array           */
/*                                                                   */
/*********************************************************************/
int add_cmds_to_history( char** cmds, int n_cmds )
{
    int ctr = 0;

    /* error handling */
    if ( history_count == CMD_LIMIT )
    {
        fprintf(stderr, 
            "At history limit. Writing history to ~/.j_history\n" );

        if ( write_history_to_file() == FAILURE )
            return FAILURE;
    }

    /* allocate memory for commands */
    if ( ( history[history_count].cmds = (char**) 
            malloc( ( n_cmds + 1 ) * sizeof(char*) ) ) == NULL )
    {
        fprintf( stderr, "Error allocating memory for history." );
        return FAILURE; 
    }

    /* set n_cmds to 0 */
    history[history_count].n_cmds = 0;

    /* add the cmds to the structure array */
    while( ctr != n_cmds )
    {
        add_string( &cmds[ctr++], &history[history_count].cmds, 
                    &history[history_count].n_cmds );
    }
    history_count++;

    return SUCCESS;
} /* end add_cmds_to_history() */


/*********************************************************************/
/*                                                                   */
/*      Function name: print_history                                 */
/*      Return type:   void                                          */
/*      Parameter(s): none                                           */
/*                                                                   */
/*********************************************************************/
void print_history( FILE *fp )
{
    int ctr = 0, i = 0; 
    while( ctr < history_count )
    {
        fprintf( fp, "\t" );
        for ( ; i < history[ctr].n_cmds; i++ )
            fprintf( fp, "%s ", history[ctr].cmds[i] );

        fprintf( fp, "\n" );
        i = 0;
        ctr++;
    }
} /* end print_history() */


/*********************************************************************/
/*                                                                   */
/*      Function name: write_history_to_file                         */
/*      Return type:   int                                           */
/*      Parameter(s): none                                           */
/*                                                                   */
/*********************************************************************/
int write_history_to_file( void )
{
    char out_file[255];
    sprintf( out_file, "%s%s", getenv( "HOME" ), "/.j_history" ); 
    char* mode = "a+";
    FILE* fp; 

    /* open file to write to */
    if( ( fp = fopen( out_file, mode ) ) == NULL )
    {
        fprintf( stderr, "Error. Could not open %s\n", out_file );
        perror("Error");
        return FAILURE;
    }

    /* write all history to outfile */
    print_history( fp ); 
    free_history();

    /* close file */
    fclose( fp );

    return SUCCESS; 
} /* end write_history_to_file() */


/*********************************************************************/
/*                                                                   */
/*      Function name: free_history                                  */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*      Description:                                                 */
/*          Frees all memory allocated for history array.            */
/*                                                                   */
/*********************************************************************/
int free_history( void )
{
    int ctr = 0, i = 0; 
    while( ctr < history_count )
    {
        for ( ; i < history[ctr].n_cmds; i++ )
        {
            free( history[ctr].cmds[i] );
            history[ctr].cmds[i] = NULL;
        }
        history[ctr].n_cmds = 0;
        free( history[ctr].cmds );
        history[ctr].cmds = NULL;

        i = 0;
        ctr++;
    }
    history_count = 0;

    return SUCCESS; 
} /* end free_history() */
