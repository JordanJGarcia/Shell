
#include "string_module.h"

/*********************************************************************/
/*                                                                   */
/*      Function name: build_string                                  */
/*      Return type:   char*                                         */
/*      Parameter(s):  1                                             */
/*          char c: char to add to command                           */
/*          char**: pointer to the string we are appending to.       */
/*                                                                   */
/*      Description:                                                 */
/*          build_command accepts a character and appends it to the  */
/*          string "cmd." It creates a string one letter at a time.  */
/*                                                                   */
/*********************************************************************/
int build_string( char c, char** cmd )
{
    //printf( "cmd = %s\n", *cmd );
    int size = ( *cmd == NULL ? 0 : strlen(*cmd) );

    if ( size > 0 )
    {
        if ( ((*cmd) = (char*)realloc( *cmd, (size + 2) * sizeof(char)) 
             ) == NULL )
            return FAILURE;
    }
    else
    {
        if ( ((*cmd) = (char*)malloc( (size + 2) * sizeof(char) )
             ) == NULL )
            return FAILURE; 
    }
 
    (*cmd)[size] = c;
    (*cmd)[size + 1] = '\0';

    return SUCCESS;
}/* end build_string() */



/*********************************************************************/
/*                                                                   */
/*      Function name: add_string                                    */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char** str: pointer to string we are adding              */
/*          char*** arr: pointer to array we are adding string to    */
/*          int* str_count: pointer to number of strings in arr.     */
/*                                                                   */
/*      Description:                                                 */
/*          adds a string to an array of strings.                    */
/*                                                                   */
/*********************************************************************/
int add_string( char** str, char*** arr, int* str_count )
{
    /* if there hasn't been a command initialized */
    if ( *str == NULL )
        return FAILURE;

    /* allocate memory to add new string to array */
    if ( *str_count == 0 )
    {
        if ( (*arr = (char**)malloc( sizeof(char*) ) ) == NULL )
            return FAILURE;
    }
    else
    {
        if ( (*arr = (char**)realloc(*arr, (*str_count + 1) 
                                     * sizeof(char*) ) ) == NULL
           )
            return FAILURE;
    }

    /* allocate memory for the actual string in the array */
    if ( ( (*arr)[*str_count] = (char*)malloc( ( strlen(*str) + 1 )
                                          * sizeof(char) ) ) == NULL
       )
        return FAILURE;

    strcpy( (*arr)[*str_count], *str );

    /* increase command count */
    *str_count += 1;

    /* allocate memory to add null term */
    if ( (*arr = (char**)realloc( *arr, (*str_count + 1) * 
                                sizeof(char*) ) ) == NULL 
       )
        return FAILURE;

    /* last element set to NULL for execv() to work */
    (*arr)[*str_count] = NULL;

    /* free up memory */
    free( *str );
    *str = NULL;

    return SUCCESS;
}/* end add_string() */


/*********************************************************************/
/*                                                                   */
/*      Function name: move_strings_down                             */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char*** arr: pointer to array we are adjusting.          */
/*          int* arr_size: size of arr.                              */
/*          int n_indices: number of indices we are moving down.     */
/*          int start: starting position of adjustment               */
/*                                                                   */
/*      Description:                                                 */
/*          moves strings from start onwards down n_indices in arr.  */
/*          removes the string *arr[start] from the array.           */
/*                                                                   */
/*********************************************************************/
int move_strings_down( char*** arr, int* arr_size, int n_indices, 
                       int start )
{
    int new_arr_size = *arr_size + n_indices;
    int old_arr_size = *arr_size - 1;
    int stop = start + n_indices;
    int end_freeable_mem = ( stop < *arr_size ? stop : *arr_size );
    int end_null_mem = new_arr_size - *arr_size + start;

    /* error allocating memory for array */
    if ( (*arr = (char**) realloc( *arr, (new_arr_size)
                * sizeof(char*) ) ) == NULL 
       )
        return FAILURE; 

    /* if the array only has one string to move */
    if ( *arr_size == 1 )
    {
        free( (*arr)[old_arr_size] );
        while( old_arr_size <= stop )
            (*arr)[old_arr_size++] = NULL;

        /* adjust arr size */
        *arr_size = new_arr_size - 1;

        return SUCCESS;
    }

    /* put strings in new indices */
    for( ; old_arr_size != start; old_arr_size-- )
    {
        /* clean up old memory */
        if ( old_arr_size + n_indices - 1 <= *arr_size - 1 )
        {
            free( (*arr)[old_arr_size + n_indices - 1] );
            (*arr)[old_arr_size + n_indices] = NULL;
        }

        /* adjust memory for new string */
        (*arr)[old_arr_size + n_indices - 1] = (char*) malloc(
                ( strlen( (*arr)[old_arr_size] ) + 1 ) *
                sizeof(char) ); 

        /* error allocating memory */
        if ( (*arr)[old_arr_size + n_indices - 1] == NULL )
            return FAILURE; 

        /* copy string into new location */
        strcpy( (*arr)[old_arr_size + n_indices - 1], 
                (*arr)[old_arr_size] );
    }

    /* remove strings from old locations */
    while( start < end_freeable_mem )
    {
        free( (*arr)[start] );
        (*arr)[start++] = NULL;   
    }

    while( start < end_null_mem )
        (*arr)[start++] = NULL;

    /* adjust arr size */
    *arr_size = new_arr_size - 1;

    /* make end of array null */
    (*arr)[*arr_size] = NULL;

    return SUCCESS;
} /* end move_strings_down() */


/*********************************************************************/
/*                                                                   */
/*      Function name: add_strings                                   */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char*** to: pointer to array we are adjusting.           */
/*          char*** from: pointer to array of strings we are adding. */
/*          int* to_size: size of arr to.                            */
/*          int start: index to start placing strings.               */
/*          int n_indices: num indices to copy to.                   */
/*          int alias: T/F value to determine if we are replacing    */
/*                     an alias.                                     */
/*                                                                   */
/*      Description:                                                 */
/*          moves strings from start onwards down n_indices in arr.  */
/*                                                                   */
/*********************************************************************/
int add_strings( char*** to, char*** from, int start, 
                 int n_indices, int alias )
{
    int stop = start + n_indices; 
    int ind = 0;

    /* place strings into arr from start */
    for( ; start < stop; start++ )
    {
        /* allocate memory for new string */
        (*to)[start] = (char*) malloc( (strlen( (*from)[ind] ) + 1) *
                                        sizeof(char) );

        if( (*to)[start] == NULL )
            return FAILURE; 

        /* copy old string into new arr */
        strcpy( (*to)[start], (*from)[ind++] );
    }

    return SUCCESS;
}