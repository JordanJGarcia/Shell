#include "execution.h"

/*********************************************************************/
/*                                                                   */
/*      Function name: execute                                       */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          executes a program entered in the command line by user.  */
/*                                                                   */
/*********************************************************************/
void execute( void )
{
    pid_t pgid = getpgrp();
    pid_t pid = 0;
    int status, w;
    void (*istat)(int), (*qstat)(int);
    int tty = open( "/dev/tty", O_RDWR );

    /* error handling if can't open /dev/tty */
    if(tty == -1)
    {
        fprintf(stderr, "Error: can't open /dev/tty\n");
        return;
    }
    
    /* if in child process */
    if( ( pid = fork() ) == 0 )
    {
        /* set process group ID */
        setpgid( 0, pgid );

        /* duplicate stdin, stdout and stderr to controlling terminal */
        dup2( tty, 0 );
        dup2( tty, 1 );
        dup2( tty, 2 );
    
        /* close controlling terminal */
        close( tty );

        /* attempt to execute program and print error if needed */
        execvp( cmds[0], cmds );

        /* execvp only returns if an error occured */
        fprintf( stderr, "Error. Can't execute %s\n", cmds[0] );

        return;
    }

    /* set process group ID */
    setpgid( pid, pgid );

    /* ignore ctrl-c & ctrl-\ */
    istat = signal(SIGINT, SIG_IGN);
    qstat = signal(SIGQUIT, SIG_IGN);

    /* close tty in parent process */
    close(tty);

    /* wait for child processes to finish */
    while( ( w = wait( &status ) ) != pid && w != -1 )
        continue;

    /* allow for ctrl-c & ctrl-\ */
    signal(SIGINT, istat);
    signal(SIGQUIT, qstat);

    return;
}/* end execute() */


/*********************************************************************/
/*                                                                   */
/*      Function name: redirect_input                                */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          executes a program entered in the command line by user   */
/*          and redirects the input into the program.                */
/*                                                                   */
/*********************************************************************/
void redirect_input( void )
{
    pid_t pgid = getpgrp();
    pid_t pid = 0;
    int status, w;
    void (*istat)(int), (*qstat)(int);
    int in_file_pos = find_string( "<", &cmds, n_cmds );
    char** exec_cmds = NULL;

    /* ensure we found output file */
    if ( in_file_pos == -1 )
    {
        fprintf( stderr, "Error: Could not find input file.\n" );
        return; 
    }

    /* open file with read access */
    int fd_in = open( cmds[in_file_pos + 1], O_RDONLY);

    /* error handling for opening a file */
    if ( fd_in == -1 )
    {
        /* print error to stderr and return */
        fprintf( stderr, "Error: Can't open file: %s\n", 
                 cmds[in_file_pos + 1] );

        return;
    }

    /* allocate memory to copy execution commands */
    exec_cmds = (char**) malloc( ( in_file_pos +1 ) 
                                        * sizeof( char* ) );

    /* ensure memory properly allocated */
    if ( exec_cmds == NULL )
    {
        fprintf(stderr, "Error, could not allocated memory for \
                         command execution in input redirect\n" );

        return;
    }

    /* copy cmds up to '<' 
       (we only want to execute the portion prior) */
    add_strings( &exec_cmds, &cmds, 0, in_file_pos );

    /* set last index to NULL for execvp */
    exec_cmds[in_file_pos] = NULL;

    /* if in child process */
    if ( ( pid = fork() ) == 0 )
    {
        /* set process group ID */
        setpgid( 0, pgid );

        /* duplicate file descriptor to stdin */
        dup2( fd_in, 0 );

        /* close file descriptor in child process, 
           because a copy of it is attached to stdout */
        close( fd_in ); 

        /* execute program */
        execvp( exec_cmds[0], exec_cmds );

        /* execvp returns only on error */
        fprintf( stderr, "can't execute %s\n", exec_cmds[0] );

        return;
    }
    
    /* set process group ID */
    setpgid( pid, pgid );

    /* ignore ctrl-c & ctrl-\ */
    istat = signal(SIGINT, SIG_IGN);
    qstat = signal(SIGQUIT, SIG_IGN);

    /* close file descripter in parent process,
        a copy is also made here in parent */
    close( fd_in );

    /* waiting for all current child processes and get their termination statuses */
    while((w = wait(&status)) != pid && w != -1)
        continue;

    /* allow for ctrl-c & ctrl-\ */
    signal(SIGINT, istat);
    signal(SIGQUIT, qstat);

    /* free memory used to create exec_cmds */
    int ctr = 0;
    for( ; ctr < in_file_pos; ctr++ )
    {
        free( exec_cmds[ctr] );
        exec_cmds[ctr] = NULL;
    }
    free( exec_cmds );

    return;
} /* end redirect_input */


/*********************************************************************/
/*                                                                   */
/*      Function name: redirect_output                               */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          executes a program entered in the command line by user   */
/*          and redirects the output.                                */
/*                                                                   */
/*********************************************************************/
void redirect_output( void )
{
    pid_t pgid = getpgrp();
    pid_t pid = 0;
    int status, w;
    void (*istat)(int), (*qstat)(int);
    int out_file_pos = find_string( ">", &cmds, n_cmds );
    char** exec_cmds = NULL;

    /* ensure we found output file */
    if ( out_file_pos == -1 )
    {
        fprintf( stderr, "Error: Could not find output file.\n" );
        return; 
    }

    /* open file with read/write access or create new file */
    int fd_out = open( cmds[out_file_pos + 1], O_RDWR | O_CREAT, 0666 );

    /* error handling for opening a file */
    if ( fd_out == -1 )
    {
        /* print error to stderr and return */
        fprintf( stderr, "Error: Can't open file: %s\n", 
                 cmds[out_file_pos + 1] );

        return;
    }

    /* allocate memory to copy execution commands */
    exec_cmds = (char**) malloc( ( out_file_pos +1 ) 
                                        * sizeof( char* ) );

    /* ensure memory properly allocated */
    if ( exec_cmds == NULL )
    {
        fprintf(stderr, "Error, could not allocated memory for \
                         command execution in input redirect\n" );

        return;
    }

    /* copy cmds up to '<' 
       (we only want to execute the portion prior) */
    add_strings( &exec_cmds, &cmds, 0, out_file_pos );

    /* set last index to NULL for execvp */
    exec_cmds[out_file_pos] = NULL;

    /* if in child process */
    if ( ( pid = fork() ) == 0 )
    {
        /* set process group ID */
        setpgid( 0, pgid );

        /* duplicate file descriptor to stdout */
        dup2( fd_out, 1 );

        /* close file descriptor in child process, 
           because a copy of it is attached to stdout */
        close( fd_out ); 

        /* execute program */
        execvp( exec_cmds[0], exec_cmds );

        /* execvp returns only on error */
        fprintf( stderr, "can't execute %s\n", exec_cmds[0] );

        return;
    }
    
    /* set process group ID */
    setpgid( pid, pgid );

    /* ignore ctrl-c & ctrl-\ */
    istat = signal(SIGINT, SIG_IGN);
    qstat = signal(SIGQUIT, SIG_IGN);

    /* close file descripter in parent process,
        a copy is also made here in parent */
    close( fd_out );

    /* waiting for all current child processes and get their termination statuses */
    while((w = wait(&status)) != pid && w != -1)
        continue;

    /* allow for ctrl-c & ctrl-\ */
    signal(SIGINT, istat);
    signal(SIGQUIT, qstat);

    /* free memory used to create exec_cmds */
    int ctr = 0;
    for( ; ctr < out_file_pos; ctr++ )
    {
        free( exec_cmds[ctr] );
        exec_cmds[ctr] = NULL;
    }
    free( exec_cmds );

    return;
} /* end redirect_output() */


/*********************************************************************/
/*                                                                   */
/*      Function name: redirect_output_and_input                     */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          executes a program entered in the command line by user   */
/*          and redirects the output and input.                      */
/*                                                                   */
/*********************************************************************/
void redirect_output_and_input( void )
{

} /* end redirect_output_and_input */






