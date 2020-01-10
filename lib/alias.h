/*********************************************************************/
/*                                                                   */
/*          Module name: alias.h                                     */
/*          Description:                                             */
/*              This module provides structures and functions to     */
/*              store and remove aliases.                            */
/*                                                                   */
/*********************************************************************/


#ifndef ALIAS_H 
#define ALIAS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string_module.h"

#define ALIAS_LIMIT 100

/* structure to hold alias values */
typedef struct alias_t
{
    char*   original;
    char**  translated;
    int     n_cmds;
} alias;

/* global variables */
char**          trans;
char*           token;  
extern int      n_cmds;

/* global variable to hold aliases */
alias   alias_arr[ALIAS_LIMIT];

/* prototypes */
alias*  add_alias( const char*, char* );
alias*  find_alias( const char* );
alias*  remove_alias( const char* );
void    parse_translated( const char* );
void    adjust_aliases( alias* found );
void    print_aliases( void );
void    place_tokens( void );
int     alias_cmp( const void*, const void* );
int     free_aliases( void );

#endif