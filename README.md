Kirsten Vail CS241, Shell in C

The included program was created for the course CS241 (Systems programming) at Oberlin College. It is a command shell.

It has the following functionality:

-input and output redirection
-single piping
-ability to run background processes
-traps SIGINT such that instead of terminating the current process (your shell) it terminates any running child process
-run single and multi word command lines via forking and then execing
-there are the following built in commands:
<br>
exit : exits the shell
myinfo : prints out PID and PPID
cd : changes current directory to $HOME using getenv() and chdir()
cd &lt; dir &gt; :changes current directory to specified directory using chdir()
