/*********************************************************************/
/*                                                                   */
/*          Module name: execution.h                                 */
/*          Description:                                             */
/*              This module provides functions to execute programs.  */
/*                                                                   */
/*********************************************************************/

#ifndef EXECUTION_H
#define EXECUTION_H

/* directives */
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "./string_module.h"

/* macros */
#define OUTPUT 1
#define INPUT -1
#define PIPE 3
#define INOUT 2
#define INOUTPIPE 4;
#define FAILURE 0
#define SUCCESS 1

/* globals */
extern char** cmds; 
extern int n_cmds;

/* function prototypes */
void    execute( void );
void    redirect_input( void );
void 	redirect_output( void );
void    redirect_both( void );
void    execute_and_pipe( void );
void 	redirect_input_and_pipe( void );
void 	redirect_output_and_pipe( void );
void 	redirect_both_and_pipe( void );

#endif