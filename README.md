# JShell - The Jordan Shell

!!!  NOT COMPLETE - Still in development  !!!

This is my implementation of a simple shell program. Here are a few things you will need before using the JShell:

1. Readline library
2. GNU C Compiler

JShell will support many basic features including:

1. Records history of typed commands
    - After 50 commands typed, command history will be written to $HOME/.j_history file. 
    - You can change the command limit in the macro CMD_LIMIT located in /lib/command_history.h
  
 2. Aliases
    - You can add aliases that exist only while JShell is running.
    - You can remove aliases that have been added during the programs lifetime.
    - NOT IMPLEMENTED YET - The shell will look through your $HOME/.j_profile file to automatically add any  aliases you have in there. 
 
3. Translation of environmental variables
    - Will translate environmental variables (if defined) whether inside quotes or not.
  
4. Change Directories
    - Will handle changing of directories.
    - "cd" typed by itself will change to $HOME directory. 
    - Can handle relative and absolute path directory changes. 
  
5. Echo
    - Will echo as expected, with one exception that I know of:
      - Due to the design of the string parser, when you place an env variable in front of a punctuation like "," or "?" JShell     will translate the env variable and place a space after so your final output will look like this: "echo $USER, how are you?" > "[user] , how are you?".
    
4. Program execution
    - This includes:
      - Pipes
      - I/O redirection
      - Background processes
      - Multiple of any of the above
      - Any combination of any of the above
    
5. Command completion
    - Please note that this is done through readline and JShell requires readline library to be installed or else the program will not compile.

My version of the executable is available in the /bin directory if you would like to download that.
However all source code is available to the public to be modified however you please. General input/criticism
is appreciated as I am always looking for ways to improve my craft. Enjoy!

To use JShell, either:

1. Download executable directly from ubin folder and run from housing directory. 

OR

1. Create a local copy of this repository in a macOS X or Linux environment (its own directory).
2. Execute "make" command. 
3. Run program with "./shell"
4. End program at any time by typing "exit" or Control-C.  
  
 Extras:
 
 There are some libraries that are used specifically for this project (lib directory) that may also be useful for other programs. For example, the string_module.h module has some useful functions for creating strings and modifying string arrays. You are free to use/improve this module however youd like. A single repository if these modules can be found in my github profile. 
