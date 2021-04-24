<h2> Assignment 1 </h2>
<h6> CS 550, Spring 2021 </h6>

---
<b> Hannah Burkhard </b>
hburkha3@binghamton.edu

<b> Crystal Low </b>
clow1@binghamton.edu

---

<h4> General notes </h4>

This zip file includes a Makefile and parser.c. We believe there are no bugs and the program runs smoothly.

<h4> How to run </h4>

To Compile the program execute: <code> make all </code>
To Remove all executables execute: <code> make clean </code>

To run the Parser execute: <code> ./parser </code>
To exit the Parser execute: <code> exit </code>

<h4>Part A</h4>

Upon execution, the program will prompt the user for input. The following prompt will display: <code> sh550> </code>.

Any generic unix commands, such as <code> ls </code>, can be executed simply by typing the command followed by any arguments into the prompt. In addition, unqiue shell scripts can be executed by <code> ./test.sh </code>. To run any commands in the background, bypassing the wait, execute with an ampersand & suffix.

To list all background jobs in the shell enter the command <code> listjobs </code>. To bring any currently running background process to the foreground enter the command <code> fg </code> followed by the PID.

<h4>Part B </h4>

The implementation of I/O redirection now allows the end user to redirect the input/output of commands from/to stdin/stdout to files.

To utilize <i>input</i> redirection, enter command in the following format:
<code> [Your command] < [name of input file]</code>

To utilize <i>output</i> redirection, enter command in the following format:
<code> [Your command] > [name of output file]</code>

The addition of filters now allows for the redirection of the standard output of one command to the standard input of another using the piping mechanism. With the implementation of a matrix, any number of filters may be utilized. The filter functionality can be executed with any of the other features, such as I/O redirection and background processes. Any filter background processes will display under listjobs accroding to the first command and it's corresponding PID.

To utilize filter functionality, enter command in the following format:

<code>[Command a] | [Command b] | .... | [Command n]</code>

For example,

<code> ls -l | wc -l </code>

<h4> Part C </h4>

The shell now allows for the terminiation of both foreground and background processes. Any of the above scenarios can be executed with the teremination functionality.

To kill <b>Foreground</b> processes press: <code> CTRL + C</code>
The current forground process will be terminated and the end user will be redirected back to the shell prompt. 

The <code> CTRL + C</code> mechanism is handled through the implentation of a signal handler on <b>SIGINT</b>

To kill <b> Background </b> processes enter: <code>kill [pid] </code>
<i>Note:</i> <code>CTRL + C</code> can ONLY be used on forground processes and <code>Kill</code> can ONLY be used on background processes.

The termination of background filtered commands can be executed by entering the following:
<code>kill [pid of child 1]</code>.
Implementing this required the utilization of functions such as <code> killpg(...) </code> and <code> setpgid(...) </code>. 
