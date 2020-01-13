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
/*      Function name: redirect_output                               */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          executes a program entered in the command line by user   */
/*          and redirects the output.                                */
/*                                                                   */
/*********************************************************************/

// I MAY NEED TO ADJUST CMDS TO UNINCLUDE THE ">" or "<" 
void redirect_output( void )
{
    pid_t pgid = getpgrp();
    pid_t pid = 0;
    int status, w;
    void (*istat)(int), (*qstat)(int);
    int out_file_pos = find_string( ">", &cmds, n_cmds );

    /* ensure we found output file */
    if ( out_file_pos == -1 )
    {
        fprintf( stderr, "Error: Could not find output file.\n" );
        return; 
    }

    /* open file with read/write access or create new file */
    int fd_out = open( cmds[out_file_pos + 1], O_RDWR | O_CREAT, 0666);

    /* error handling for opening a file */
    if ( fd_out == -1 )
    {
        /* print error to stderr and return */
        fprintf( stderr, "Error: Can't open file: %s\n", 
                 cmds[out_file_pos + 1] );

        return;
    }

    /* if in child process */
    if ( ( pid = fork() ) == 0 )
    {
        /* set process group ID */
        setpgid( 0, pgid );

        /* duplicate stdout to file descriptor */
        dup2( fd_out, 1 );

        /* close file descriptor in child process, 
           attaching it to stdout */
        close( fd_out ); 


        // I BELIEVE THAT THIS EXECVP IS GIVING ME AN ERROR WHEN EXECUTING OUTPUT REDIRECTION.

        /* execute program */
        execvp( cmds[0], cmds );

        /* execvp returns only on error */
        fprintf( stderr, "can't execute %s\n", cmds[0] );

        return;
    }
    
    /* set process group ID */
    setpgid( pid, pgid );

    /* ignore ctrl-c & ctrl-\ */
    istat = signal(SIGINT, SIG_IGN);
    qstat = signal(SIGQUIT, SIG_IGN);

    /* close file descripter in parent process */
    close( fd_out );

    /* waiting for all current child processes and get their termination statuses */
    while((w = wait(&status)) != pid && w != -1)
        continue;

    /* allow for ctrl-c & ctrl-\ */
    signal(SIGINT, istat);
    signal(SIGQUIT, qstat);

    return;
} /* end redirect_output() */