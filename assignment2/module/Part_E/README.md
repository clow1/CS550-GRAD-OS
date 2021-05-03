<h2> Assignment 2 </h2>
<h6> CS 550, Spring 2021 </h6>

---
<b> Hannah Burkhard </b>
hburkha1@binghamton.edu

<b> Crystal Low </b>
clow1@binghamton.edu

---

<h4> General notes </h4>

This zip file includes proceess_list.c (a kernel module which creates a process_list character device) user_space_prog.c (the user space program which invokes the read operation from within the process_list character device), a Makefile, and a sample output. We believe there are no bugs and the program runs correctly.

<h4> How to run </h4>

To Compile the program execute: <code> make </code>

To Insert the kernel module execute: <code> sudo insmod process_list.ko </code>

To Run the user space program execute: <code> sudo ./user_space_program </code>

To Remove the kernel module execute: <code> sudo rmmod process_list </code>

To Remove all executables execute: <code> make clean </code>

<h4> Kernel Module </h4>
Once the kernel module, process_list, has been inserted into the kernel space the read() operation can be performed to return the following information reguarding all running process:

- process ID

- parrent process ID

- the CPU the process is running on

- the current state

The user program invokes the read() operation, described above, and outputs the retrieved data to standard out. The program's output is printed in the following format:

PID=[Process_ID] PPID=[Parrent_PID] CPU=[CPU_Running] STATE=[Current_State]
