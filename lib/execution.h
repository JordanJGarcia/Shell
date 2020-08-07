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
#define READ_END 0
#define WRITE_END 1

/* globals */
extern char** cmds; 
extern int n_cmds;

/* function prototypes */
int     generate_process( int fd_in, int fd_out, char*** prog );
int 	generate_process_for_pipe( int fd_in, int fd_out, char*** prog );

/* standard program execution */
void    execute( void );

/* I/O redirections */
void    redirect_input( void );
void    redirect_output( void );
void    redirect_input_and_output( void );

/* standard pipelines */
void    execute_and_pipe( int );

/* pipelines and redirections */
void    redirect_input_and_pipe( void );
void    redirect_output_and_pipe( void );
void    redirect_both_and_pipe( void );

#endif