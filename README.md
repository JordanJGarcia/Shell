# JShell - The Jordan Shell

This is my implementation of a simple shell program. It supports many basic features including:

1. Aliases
  - You can add aliases that exist only while program is running.
  - You can remove aliases.
  - NOT IMPLEMENTED YET - The shell will look through your .profile file to automatically add any aliases you have in there. 
  
2. Program Execution
3. I/O Redirection
4. Pipes
5. Command completion (you must have readline library installed)

My version of the executable is available in the /ubin folder if you would like to download that.
However all source code is available to the public to be modified however you please. General input/criticism
is appreciated as I am always looking for ways to improve my craft. Enjoy!

To compile:

1. Create a local copy of this repository in a macOS X or Linux environment (its own directory).
2. Either run the "make" command or run "gcc -o shell shell.c alias.c -lreadline"
3. Run program with "./shell"
4. End program at any time with "exit" command. 

Current Bugs:

1. When printing out strings with "echo," this shell will place a space between previous token and the punction that comes    after. 
  - Ex: echo "Hello, world" || echo Hello, world" will print "Hello , world"
  
 Extras:
 
 There are some libraries that are used specifically for this project (lib directory) that may also be useful for other programs. For example, the string_module.h module has some useful functions for creating strings and modifying string arrays. You are free to use/improve this module however youd like. A single repository if these modules can be found in my github profile. 
