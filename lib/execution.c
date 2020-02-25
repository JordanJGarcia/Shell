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

    /* set last index of commands 
       (before redirection operator)to NULL for execvp */
    cmds[in_file_pos] = NULL;

    /* spawn process and execute prog */
    pid = generate_process( fd_in, 1, &cmds );

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
    pid = generate_process( 0, fd_out, &cmds );

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
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          executes a program entered in the command line by user   */
/*          and creates a pipeline to another program.               */
/*                                                                   */
/*********************************************************************/

    // divide programs into sepearate char arrays
    // send each program through pipe 
void execute_and_pipe( int pos )
{
    char** current_program = cmds;
    int i, pid, fd_in, pipe_fd[2];
    int num_pipes =  count_pipes();
    int pipe_location = 0, prog_cmd_count = n_cmds;
    printf( "Number of pipes = %d\n", num_pipes );

    /* first process gets its input from stdin */
    fd_in = 0;

    for( i = 0; i < num_pipes; i++ )
    {
        /* get location of pipe and set it to null */
        pipe_location = find_string( "|", &current_program, prog_cmd_count );

        if ( pipe_location != -1 )
            cmds[pipe_location] = NULL;
        else
            return;

        /* create pipeline */
        if ( pipe( pipe_fd ) == -1 )
        {
            /* error handling */
            fprintf( stderr, "Error: pipe failed.\n" );
            return;
        } /* pipe has been created */

        pid = generate_process( fd_in, pipe_fd[1], &current_program );

        /* set current program as next program */
        current_program = &current_program[pipe_location + 1];
        prog_cmd_count -= ( pipe_location + 1 );

        fd_in = pipe_fd[0];
    }

    /* set stdin to be the read end of the pipe */
    if ( fd_in != 0 )
        dup2( fd_in, 0 );

    /* execute last program in pipeline */
    execvp( current_program[0], current_program );
    
    /* execvp returns only on error */
    fprintf( stderr, "Error: Can't execute %s\n", current_program[0] );

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
        /* if we are not getting from stdin, reassign input. */
        if ( fd_in != 0 )
        {
            dup2( fd_in, 0 );
            close( fd_in );
        }

        /* if we are not directing to stdout, reassign output */
        if ( fd_out != 1 )
        {
            dup2( fd_out, 1 );
            close( fd_out );
        }

        //puts( "in child process..." );

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

    //puts( "in parent process...." );

    return pid;
}


/*********************************************************************/
/*                                                                   */
/*      Function name: count_pipes                                   */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          counts occurences of pipes in cmds.                      */
/*                                                                   */
/*********************************************************************/
int count_pipes( void )
{
    int old_result;
    int result = find_string( "|", &cmds, n_cmds );
    int ctr = 0;

    while ( result != -1 )
    {
        ctr++;
        char** new_start = &cmds[result + 1];

        old_result = result; 
        result = find_string( "|", &new_start, n_cmds - result - 1 ); 
        result = ( result == -1 ? -1 : result + old_result + 1 );
    }
    return ctr;
}


