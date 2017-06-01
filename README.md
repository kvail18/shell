Kirsten Vail CS241, Shell in C

The included program was created for the course CS241 (Systems programming) at Oberlin College. It is a command shell.

It has the following functionality:

-input and output redirection <br>
-single piping <br>
-ability to run background processes <br>
-traps SIGINT such that instead of terminating the current process (your shell) it terminates any running child process <br>
-run single and multi word command lines via forking and then execing <br>
-there are the following built in commands:
<br>
exit : exits the shell <br>
myinfo : prints out PID and PPID <br>
cd : changes current directory to $HOME using getenv() and chdir() <br>
cd &lt; dir &gt; :changes current directory to specified directory using chdir() <br>
