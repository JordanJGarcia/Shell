#include "execution.h"


// REDESIGN IDEAS:
    // maybe turn redirection output and input functions into just one and pass a char (i/o)
    // you can include both into there with a third char (b)
    // if its b, then open second file
    // maybe abstract it further into one execute function wiht 4 chars (n - neither,i - input, o - output, b - both )
        // this one may not be a good idea. 


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
    int pid;

    /* attempt to open /dev/tty */
    int tty = open( "/dev/tty", O_RDWR );

    /* error handling if can't open /dev/tty */
    if(tty == -1)
    {
        fprintf(stderr, "Error: can't open /dev/tty\n");
        return;
    }
    
    /* spawn process and execute prog */
    pid = generate_process( tty, tty, &cmds );

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
    int pid;
    int in_file_pos = find_string( "<", &cmds, n_cmds );

    /* ensure we found input file */
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

    /* set last index to NULL for execvp */
    cmds[in_file_pos] = NULL;

    /* spawn process and execute prog */
    pid = generate_process( fd_in, STDOUT_FILENO, &cmds );

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
    int pid;
    int out_file_pos = find_string( ">", &cmds, n_cmds );

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

    /* set last index to NULL for execvp */
    cmds[out_file_pos] = NULL;

    /* spawn process and run program */
    pid = generate_process( STDIN_FILENO, fd_out, &cmds );

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
void redirect_input_and_output( void )
{
    pid_t pgid = getpgrp();
    pid_t pid = 0;

    int status, w;
    void (*istat)(int), (*qstat)(int);

    int out_file_pos = find_string( ">", &cmds, n_cmds );
    int in_file_pos = find_string( "<", &cmds, n_cmds );
    int first_operator_pos = ( in_file_pos < out_file_pos ? 
                               in_file_pos : out_file_pos );

    /* ensure we found output file */
    if ( out_file_pos == -1 )
    {
        fprintf( stderr, "Error: Could not find output file.\n" );
        return; 
    }

    /* ensure we found input file */
    if ( in_file_pos == -1 )
    {
        fprintf( stderr, "Error: Could not find input file.\n" );
        return;
    }

    /* open input and output files */
    int fd_out = open( cmds[out_file_pos + 1], O_RDWR | O_CREAT, 0666 );
    int fd_in = open( cmds[in_file_pos + 1], O_RDONLY);

    /* error handling for opening output file */
    if ( fd_out == -1 )
    {
        /* print error to stderr and return */
        fprintf( stderr, "Error: Can't open file: %s\n", 
                 cmds[out_file_pos + 1] );

        return;
    }

    /* error handling for opening input file */
    if ( fd_in == -1 )
    {
        /* print error to stderr and return */
        fprintf( stderr, "Error: Can't open file: %s\n", 
                 cmds[in_file_pos + 1] );

        return;
    }

    /* set last index to NULL for execvp */
    cmds[first_operator_pos] = NULL;

    /* spawn process and execute program */
    pid = generate_process( fd_in, fd_out, &cmds );

    return;
} /* end redirect_output_and_input */


/*********************************************************************/
/*                                                                   */
/*      Function name: execute_and_pipe                              */
/*      Return type:   void                                          */
/*      Parameter(s):                                                */
/*          int n_pipes: number of pipes entered in command line.    */
/*                                                                   */
/*      Description:                                                 */
/*          executes a program entered in the command line by user   */
/*          and creates pipelines through the programs.              */
/*                                                                   */
/*********************************************************************/
void execute_and_pipe( int n_pipes )
{
    char** current_program = cmds;
    int pipe_loc, n_full_cmds = n_pipes + 1, n_words = n_cmds;
    pid_t pid, pgid = getpgrp();
    int i, pipe_fd[n_pipes][2];
    int status, w;
    void (*istat)(int), (*qstat)(int);

    /* create pipeline */
    if ( pipe( pipe_fd[0] ) == -1 )
    {
        /* error handling */
        fprintf( stderr, "Error: Calling pipe() failed.\n" );
        return;
    } /* pipe has been created */

    /* process programs */
    for ( i = 0; i < n_pipes; i++ )
    {
        pipe_loc = find_string( "|", &current_program, n_words );

        /* check that we found pipe_loc */
        if ( pipe_loc == -1 && i != n_pipes - 1 )
        {
            fprintf( stderr, "Error: Could not get location of next pipe.\n" );
            return;
        }

        /* adjust amount of words for next current_program */
        n_words = n_words - pipe_loc; 

        /* set pipe_loc to null so execvp knows where to stop */
        current_program[pipe_loc] = NULL;

        /* first cmd running */ 
        if ( i == 0 )
        {
            /* create child process */
            if( ( pid = fork() ) == 0 )
            {
                /* duplicate write end of pipe to stdout */
                dup2( pipe_fd[0][WRITE_END], STDOUT_FILENO );

                /* close pipe in child now that its been duped */
                close( pipe_fd[0][READ_END] );
                close( pipe_fd[0][WRITE_END] );

                /* stdin not affected yet, run program */
                execvp( current_program[0], current_program );
            } /* parent process */

            /* wait for child processes to finish */
            while( ( w = wait( &status ) ) != pid && w != -1 )
                continue;
        }
        else
        {
            /* create another pipeline */
            if ( pipe( pipe_fd[i] ) == -1 )
            {
                /* error handling */
                fprintf( stderr, "Error: Calling pipe() failed.\n" );
                return;
            } /* pipe has been created */

            /* create child process */ 
            if( ( pid = fork() ) == 0 )
            {
                /* set stdout of current cmd to write end of current pipe */
                /* set stdin of current cmd to read end of previous pipe */
                dup2( pipe_fd[i-1][READ_END], STDIN_FILENO );
                dup2( pipe_fd[i][WRITE_END], STDOUT_FILENO );

                /* close all pipes in child now that its been duped */
                int counter = 0; 
                for ( ; counter <= i; counter++ )
                {
                    close( pipe_fd[counter][READ_END] );
                    close( pipe_fd[counter][WRITE_END] );
                }

                /* execute program */
                execvp( current_program[0], current_program );
            }
        }

        /* adjust current_program to point to next cmd */
        current_program = &current_program[pipe_loc + 1];
    }

    /* run final command */
    if( ( pid = fork() ) == 0 )
    {
        /* set stdin of first cmd to write end of last pipe */
        dup2( pipe_fd[n_pipes - 1][READ_END], STDIN_FILENO );

        /* close all pipes in child now that its been duped */
        int ctr = 0; 
        for ( ; ctr < n_pipes; ctr++ )
        {
            close( pipe_fd[ctr][READ_END] );
            close( pipe_fd[ctr][WRITE_END] );
        }

        /* execute last program */
        execvp( current_program[0], current_program );
    }

    /* close all pipes in parent */
    int ctr = 0; 
    for ( ; ctr < n_pipes; ctr++ )
    {
        close( pipe_fd[ctr][READ_END] );
        close( pipe_fd[ctr][WRITE_END] );
    }

    /* wait for child processes to finish */
    while( ( w = wait( &status ) ) != pid && w != -1 )
        continue;

    return;
} /* end execute_and_pipe */



/*********************************************************************/
/*                                                                   */
/*      Function name: generate_process                              */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          creates a process and executes a program.                */
/*                                                                   */
/*********************************************************************/
int generate_process( int fd_in, int fd_out, char*** prog )
{
    pid_t pgid = getpgrp();
    pid_t pid; 
    int status, w;
    void (*istat)(int), (*qstat)(int);

    /* if in child process */
    if( ( pid = fork() ) == 0 )
    {
        /* if we are not directing to stdout, reassign output */
        if ( fd_out != 1 )
        {
            dup2( fd_out, STDOUT_FILENO );
            close( fd_out );
        }

        /* if we are not getting from stdin, reassign input. */
        if ( fd_in != 0 )
        {
            dup2( fd_in, STDIN_FILENO );
            close( fd_in );
        }

        return execvp( (*prog)[0], (*prog) );
    } /* parent process */

    /* set process group ID */
    setpgid( pid, pgid );

    /* ignore ctrl-c & ctrl-\ */
    istat = signal(SIGINT, SIG_IGN);
    qstat = signal(SIGQUIT, SIG_IGN);

    /* close descriptors if necessary in parent */
    if ( fd_in != 0 )
        close( fd_in );

    if ( fd_out != 1 )
        close( fd_out );

    /* wait for child processes to finish */
    while( ( w = wait( &status ) ) != pid && w != -1 )
        continue;

    /* allow for ctrl-c & ctrl-\ */
    signal(SIGINT, istat);
    signal(SIGQUIT, qstat);

    return pid;
} /* end generate_process */