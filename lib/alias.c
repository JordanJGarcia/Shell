#include "alias.h"

/* globals */
int     n_aliases = 0;
int     n_tokens = 0; 

/*********************************************************************/
/*                                                                   */
/*      Function name: add_alias                                     */
/*      Return type:   alias*                                        */
/*      Parameter(s):                                                */
/*          char* og:   original values                              */
/*          char* trns: translated values                            */
/*                                                                   */
/*********************************************************************/
alias* add_alias( const char* og, char* trns )
{
    /* create an array of strings of trns */
    parse_translated( trns );

    if ( n_aliases == ALIAS_LIMIT )
    {
        fprintf( stderr, "Maximum number of allowed aliases reached.\n" );
        return NULL;
    }

    /* if alias already exists */
    if ( find_alias( og ) != NULL )
    {
        fprintf( stderr, "Alias already exists.\n" );
        return NULL;
    }

    /* allocate memory to store original */
    if ( ( alias_arr[n_aliases].original = (char*) 
            malloc( (strlen( og ) + 1) * sizeof(char) ) ) == NULL )
    {
        fprintf( stderr, "Error allocating memory for alias." );
        return alias_arr; 
    }

    strcpy( alias_arr[n_aliases].original, og );

    /* allocate memory to store translated */
    if ( ( alias_arr[n_aliases].translated = (char**) 
            malloc( (n_tokens + 1) * sizeof(char*) ) ) == NULL )
    {
        fprintf( stderr, "Error allocating memory for alias." );
        return alias_arr;
    }

    /* place tokens into alias translated */
    place_tokens();

    /* sort array for bsearch() */
    qsort( alias_arr, (size_t)(n_aliases + 1), sizeof(alias_arr[0]), alias_cmp );

    return &alias_arr[n_aliases++];
} /* end add_alias() */


/*********************************************************************/
/*                                                                   */
/*      Function name: remove_alias                                  */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*********************************************************************/
alias* remove_alias( const char* og )
{
    alias* found = find_alias( og );
    if ( found == NULL )
    {
        fprintf( stderr, "Error. Alias does not exist.\n" );
        return NULL;
    }

    alias* dummy = found; 
    adjust_aliases( dummy );
    n_aliases--;

    return found;
} /* end remove_alias() */


/*********************************************************************/
/*                                                                   */
/*      Function name: find_alias                                    */
/*      Return type:   alias*                                        */
/*      Parameter(s):                                                */
/*      char* og:  alias to find                                     */
/*                                                                   */
/*********************************************************************/
alias* find_alias( const char* og )
{

    alias* found = NULL;
    found = (alias*)bsearch( &og, alias_arr, (size_t)n_aliases, 
                             sizeof( alias_arr[0] ), alias_cmp );

    return found;
} /* end find_alias() */


/*********************************************************************/
/*                                                                   */
/*      Function name: parse_translated                              */
/*      Return type:   char*                                         */
/*      Parameter(s):                                                */
/*      char* trns: translated alias value to separate               */
/*                                                                   */
/*********************************************************************/
void parse_translated( const char* line )
{
    int line_size = strlen( line );

    for ( int i = 0; i < line_size; i++ )
    {   
        if ( i == line_size - 1 ) /* end of line */
        {
            if ( !isspace( line[i] ) )
                build_string( line[i], &token );

            add_string( &token, &trans, &n_tokens );
        }
        else if ( isspace( line[i] ) ) /* spacing */
            add_string( &token, &trans, &n_tokens );
        else /* everything else */
            build_string( line[i], &token );
    }
    return;
} /* end parse_translated() */


/*********************************************************************/
/*                                                                   */
/*      Function name: adjust_aliases                                */
/*      Return type:   alias*                                        */
/*      Parameter(s):                                                */
/*          char* found: starting point to move values down          */
/*                                                                   */
/*      Description:                                                 */
/*          Adjusts the array of aliases when removing an alias by   */
/*          moving all respective alias instances down an index.     */
/*                                                                   */
/*********************************************************************/
void adjust_aliases( alias* found )
{
    /* move all values down one index in the array */
    for ( ; found != &alias_arr[n_aliases-1]; found++ )
    {
        /* replace original with next original */

        /* free memory for current original */
        free( found->original );
        found->original = NULL;

        /* moving next original into current original */
        if ( (found->original = (char*) malloc(
            (strlen((found+1)->original)) * sizeof(char))) == NULL
          )
        {
            fprintf( stderr,
                "Error allocating memory for alias->original \
                in adjustment" );
            return;
        }

        /* copy original value */
        strcpy( found->original, (found+1)->original );

        /* replace translated with next translated */

        /* freeing memory for current translated */
        for ( int i = 0; i < found->n_cmds; i++ )
        {
            free( found->translated[i] );
            found->translated[i] = NULL;
        }

        free( found->translated );
        found->translated = NULL;

        /* moving next translated into current translated */
        if ( (found->translated = (char**) malloc(
            ((found+1)->n_cmds) * sizeof(char*))) == NULL
           )
        {
            fprintf( stderr,
                "Error allocating memory for alias->translated \
                in adjustment" );
            return;
        }       

        /* copy commands down an index */
        for ( int i = 0; i < (found+1)->n_cmds; i++ )
        {
            //free( found->translated[i] );
            if ( (found->translated[i] = (char*) malloc( 
                    strlen((found+1)->translated[i]) + 1 )) == NULL
               ) 
            {
                fprintf( stderr, 
                    "Error allocating memory in adjustment\n" );

                return;
            }
            strcpy( found->translated[i], (found+1)->translated[i] );
        }

        /* adjust n_cmds */
        found->n_cmds = (found+1)->n_cmds;
    }

    /* free up memory of last element */
    free( found->original );
    for ( int i = 0; i < found->n_cmds; i++ )
    {
        free( found->translated[i] );
        found->translated[i] = NULL;
    }

    free( found->translated );
    found->original = NULL;
    found->translated = NULL;

    return;
} /* end adjust_aliases */

/*********************************************************************/
/*                                                                   */
/*      Function name: print_aliases                                 */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*********************************************************************/
void print_aliases( void )
{
    puts( " " );
    int counter = 0;
    int trans_counter = 0;

    if ( n_aliases == 0 )
    {
        puts( "No aliases have been created." );
        return;
    }

    while ( counter != n_aliases )
    {
        printf( "alias\t%s\t", alias_arr[counter].original );
        for( trans_counter = 0; trans_counter < alias_arr[counter].n_cmds; 
             trans_counter++ )
        {
            printf( "%s ", alias_arr[counter].translated[trans_counter] );
        }
        puts( " " );
        counter++;
    }
    puts( " " );
} /* end print_aliases */


/*********************************************************************/
/*                                                                   */
/*      Function name: place_tokens                                  */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*********************************************************************/
void place_tokens( void )
{
    /* place tokens into alias translated */
    for( int i = 0; i < n_tokens; i++ )
    {
        if ( ( alias_arr[n_aliases].translated[i] = (char*) 
            malloc( (strlen(trans[i]) + 1) * sizeof(char) ) ) == NULL ) 
        {
            fprintf( stderr, "Error allocating memory for token." );
            return;
        }
        strcpy( alias_arr[n_aliases].translated[i], trans[i] );
        free(trans[i]);
        trans[i] = NULL;
    }

    free( trans );
    trans = NULL;
    alias_arr[n_aliases].n_cmds = n_tokens;
    n_tokens = 0; 

    return;
} /* end place_tokens() */


/*********************************************************************/
/*                                                                   */
/*      Function name: alias_cmp                                     */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          a1: pointer to first alias.                              */
/*          a2: pointer to second alias.                             */
/*      Description:                                                 */
/*          used to compare aliases for qsort and bsearch.           */
/*                                                                   */
/*********************************************************************/
int alias_cmp( const void* a1, const void* a2 )
{
    return strcmp( ((const alias*)a1)->original, ((const alias*)a2)->original );
} /* end alias_cmp() */


/*********************************************************************/
/*                                                                   */
/*      Function name: free_aliases                                  */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*      Description:                                                 */
/*          frees all memory allocated for aliases.                  */
/*                                                                   */
/*********************************************************************/
int free_aliases( void )
{
    int ctr = 0, i = 0; 
    while( ctr < n_aliases )
    {
        for ( ; i < alias_arr[ctr].n_cmds; i++ )
        {
            free( alias_arr[ctr].translated[i] );
            alias_arr[ctr].translated[i] = NULL;
        }
        alias_arr[ctr].n_cmds = 0;
        free( alias_arr[ctr].translated );
        free( alias_arr[ctr].original );
        alias_arr[ctr].translated = NULL;
        alias_arr[ctr].original = NULL;

        i = 0;
        ctr++;
    }
    n_aliases = 0;

    return SUCCESS; 
} /* end free_aliases() */

