/*********************************************************************/
/*                                                                   */
/*          Module name: command_history.h                           */
/*          Description:                                             */
/*              This module provides structures and functions to     */
/*              store commands so we can keep track of what user     */
/*              enters.                                              */
/*                                                                   */
/*********************************************************************/

// NEED TO FREE MEMORY FROM HERE IN SHELL.C

#ifndef CMD_HISTORY_H 
#define CMD_HISTORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string_module.h"

/* macros */
#define CMD_LIMIT 50
#define FAILURE 0
#define SUCCESS 1

/* structure to hold command input history */
typedef struct cmd_history_t
{
    int     n_cmds; 
    char**  cmds;
} cmd_history;

/* globals */
cmd_history     history[CMD_LIMIT];

/* function prototypes */
int     add_cmds_to_history( char**, int );
void    print_history( FILE* );
int     write_history_to_file( void );
int     free_history( void );

#endif