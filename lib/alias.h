#ifndef ALIAS_H 
#define ALIAS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ALIAS_LIMIT 75

/* structure to hold alias values */
typedef struct alias_h
{
    char* 	original;
    char** 	translated;
    int 	n_cmds;
} alias;

/* global variable to hold translated alias */
char** 	trans;
char* 	token; 

/* global variable to hold aliases */
alias alias_arr[ALIAS_LIMIT];

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

#endif