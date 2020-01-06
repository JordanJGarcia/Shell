# JShell - The Jordan Shell

This is my implementation of a simple shell. It supports many basic features including:

1. Aliases
2. Program Execution
3. I/O Redirection
4. Pipes
5. Command completion

My version of the executable is available in the /ubin folder if you would like to download that.
However all source code is available to the public to be modified however you please. General input/criticism
is appreciated as I am always looking for ways to improve my craft. Enjoy!

To compile:

1. Create a local copy of this repository in a macOS X or Linux environment (its own directory).
2. Either run the "make" command or run "gcc -o shell shell.c alias.c -lreadline"
3. Run program with "./shell"
4. End program at any time with "exit" command. 