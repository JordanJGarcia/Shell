#include "command_history.h"

/* globals */
int history_count = 0;

/*********************************************************************/
/*                                                                   */
/*      Function name: add_cmds_to_history                           */
/*      Return type:   int	                                         */
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
		fprintf( stderr, "Error. History is full.\n" );
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
/*      Function name: print_history	                             */
/*      Return type:   void	                                         */
/*      Parameter(s): none											 */
/*                                                                   */
/*********************************************************************/
void print_history( void )
{
	int ctr = 0, i = 0; 
	while( ctr < history_count )
	{
		printf( "\t" );
		for ( ; i < history[ctr].n_cmds; i++ )
        	printf( "%s ", history[ctr].cmds[i] );

        puts( " " );
        i = 0;
        ctr++;
    }
}
