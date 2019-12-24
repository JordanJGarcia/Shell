
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ALIAS_LIMIT 150

/* structure to hold alias values */
typedef struct alias_h
{
    char* 	original;
    char** 	translated;
    int 	n_cmds;
} alias;

/* global variable to hold aliases */
alias alias_arr[ALIAS_LIMIT];
int n_aliases = 0;

/* global variable to hold translated alias */
char** 	trans;
char* 	token; 
int 	n_tokens = 0; 

/* prototypes */
alias*  add_alias( const char*, char* );
alias* 	find_alias( const char* );
alias*  remove_alias( const char* );
void 	parse_translated( const char* );
void 	adjust_aliases( alias* found );
void 	print_aliases( void );
void 	add_cmd( void );
void 	build_token( char c );
void 	place_tokens( void );


int main( void )
{
	add_alias( "ll", "ls -i" );
	add_alias( "lrt", "ls -l -r -t" );
	add_alias( "rm" , "rm -i" );
	add_alias( "tsrc", "cd /t/s/r/c" );
	add_alias( "dsrc", "cd /d/s/r/c" );
	print_aliases();

	alias* found = find_alias("ll");

	if( found != NULL )
		puts( "alias found!" );

	//printf("found.translated = %s\n\n", found->translated );

	printf( "attempting to remove ll\n" );
	remove_alias( "ll" );
	printf( "attempting to remove tsrc\n" );
	remove_alias( "tsrc" );
	print_aliases();


    return EXIT_SUCCESS;
}

/*********************************************************************/
/*                                                                   */
/*      Function name: add_alias                                     */
/*      Return type:   alias*                                        */
/*      Parameter(s):                                                */
/* 			char* og: 	original value 								 */
/* 			char* trns: translated values 							 */
/*                                                                   */
/*********************************************************************/
alias* add_alias( const char* og, char* trns )
{	
	/* create an array of strings of trns */
	parse_translated( trns );

	if( n_aliases == ALIAS_LIMIT )
	{
		fprintf( stderr, "Maximum number of allowed aliases reached.\n" );
		return NULL;
	}

	/* if alias already exists */
	if( find_alias( og ) != NULL )
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

    return &alias_arr[n_aliases++];
}


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
	if( found == NULL )
		return NULL;

	alias* dummy = found; 
	adjust_aliases( dummy );
	n_aliases--;

	return found;
}


/*********************************************************************/
/*                                                                   */
/*      Function name: find_alias                                    */
/*      Return type:   alias*                                        */
/*      Parameter(s):                                                */
/*			char* og: alias to find 								 */
/*                                                                   */
/*********************************************************************/
alias* find_alias( const char* og )
{
	for( int i = 0; i < n_aliases; i++ )
	{
		if( strcmp( og, alias_arr[i].original ) == 0 )
			return &alias_arr[i];
	}
	return NULL;
}


/*********************************************************************/
/*                                                                   */
/*      Function name: parse_translated                              */
/*      Return type:   char*                                         */
/*      Parameter(s):                                                */
/*			char* trns: translated alias value to separate 		     */
/*                                                                   */
/*********************************************************************/
void parse_translated( const char* line )
{
	int line_size = strlen( line );

    for( int i = 0; i < line_size; i++ )
    {
        if ( i == line_size - 1 ) /* end of line */
        {
            if( !isspace( line[i] ) )
                build_token( line[i] );

            add_cmd();
        }
        else if ( isspace( line[i] ) ) /* spacing */
            add_cmd();
        else /* everything else */
            build_token( line[i] );
    }

	return;
}


/*********************************************************************/
/*                                                                   */
/*      Function name: adjust_aliases                                */
/*      Return type:   alias*                                        */
/*      Parameter(s):                                                */
/*			char* found: starting point to move values down 		 */
/*                                                                   */
/*********************************************************************/
void adjust_aliases( alias* found )
{
	/* move all values down one index in the array */
	for( ; found != &alias_arr[n_aliases-1]; found++ )
	{
		/* replace original with next original */
		free( found->original );
		found->original = NULL;

		if( (found->original = (char*) malloc(
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
		for( int i = 0; i < found->n_cmds; i++ )
		{
			free( found->translated[i] );
			found->translated[i] = NULL;
		}

		free( found->translated );
		found->translated = NULL;

		if( (found->translated = (char**) malloc(
				((found+1)->n_cmds) * sizeof(char*))) == NULL
		  )
		{
			fprintf( stderr,
				 "Error allocating memory for alias->translated \
				  in adjustment" );
			return;
		}		

		/* copy commands down an index */
		for( int i = 0; i < (found+1)->n_cmds; i++ )
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
	for( int i = 0; i < found->n_cmds; i++ )
	{
		free( found->translated[i] );
		found->translated[i] = NULL;
	}

	free( found->translated );
	found->original = NULL;
	found->translated = NULL;

	return;
}

/*********************************************************************/
/*                                                                   */
/*      Function name: print_aliases                                 */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*********************************************************************/
void print_aliases( void )
{
	int counter = 0;

	while( counter != n_aliases )
	{
		puts( "***************************************************" );
		printf( "Alias # %d: \n", counter + 1 );
		printf( "Original:\t%s\n", alias_arr[counter].original );
		printf( "Translated:\t" ); //, alias_arr[counter].translated );
		for( int i = 0; i < alias_arr[counter].n_cmds; i++ )
			printf( "%s ", alias_arr[counter].translated[i] );

		puts( " " );
		puts( "***************************************************" );
		puts( " " );
		counter++;
	}
}

/*********************************************************************/
/*                                                                   */
/*      Function name: add_command                                   */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*********************************************************************/
void add_cmd( void )
{
    /* if there hasn't been a command initialized */
    if( token == NULL )
        return;

    /* allocate memory to add new string to array */
    if( n_tokens == 0 )
        trans = (char**)malloc( sizeof(char*) );
    else
        trans = (char**)realloc( trans, (n_tokens + 1) 
                                    * sizeof(char*) );

    /* allocate memory for the actual string in the array */
    trans[n_tokens] = (char*)malloc( strlen(token) + 1 );
    strcpy( trans[n_tokens], token );

    /* increase command count */
    n_tokens += 1;

    /* allocate memory to add null term */
    trans = (char**)realloc( trans, (n_tokens + 1) * 
                                sizeof(char*) );

    /* last element set to NULL for execv() to work */
    trans[n_tokens] = NULL;

    /* free up memory */
    free( token );
    token = NULL;

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
void build_token( char c )
{
    int size = ( token == NULL ? 0 : strlen(token) );

    if( size > 0 )
        token = (char*)realloc( token, (size + 2) * sizeof(char) );
    else
        token = (char*)malloc( (size + 2) * sizeof(char) );

    token[size] = c;
    token[size + 1] = '\0';
 
    return;
}/* end build_command() */


/*********************************************************************/
/*                                                                   */
/*      Function name: place_tokens                                  */
/*      Return type:   void                                          */
/*      Parameter(s):  none											 */
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
}
