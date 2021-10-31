Description: 
When executed, this program emulates a new shell similar to a bash shell. General Syntax for command line is: 
    command [arg1 arg2 ...] [< input_file] [> output_file] [&]
Where commands in [] are optional and the '&' operator specifies that command will be run in the background. If Control-Z (SIGTSTP) is raised, '&' will be ignored and all commands will be run in the foreground. Raising SIGTSTP again will allow users to run background processes again. If Control-C (SIGINT) is raised, the foreground process will be terminated but all background processes will ignore the signal and continue.

Command to compile to create smallsh executable:
    gcc --std=gnu99 -o smallsh  main.c

Command to run smallsh executable:
    ./smallsh
