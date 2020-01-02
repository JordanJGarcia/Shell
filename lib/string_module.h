/*********************************************************************/
/*                                                                   */
/*      Module for manipulating strings and string arrays            */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

#ifndef STRING_MOD_H
#define	STRING_MOD_H

/* libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* macros */
#define FAILURE 0
#define SUCCESS 1
#define T 1
#define F 0

/* function prototypes */
int 	build_string( char, char** );
int     add_string( char**, char***, int* );
int		add_strings( char***, char***, int, int );
int 	move_strings_down( char***, int*, int, int );

#endif