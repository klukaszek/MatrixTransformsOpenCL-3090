CIS*3090 Assignment 4
Name: Kyle Lukaszek
ID: 1113798
Due: November 24th, 2023

COMPILATION INSTRUCTIONS:

To compile the program, run the following command:
  make

You should now have an executable called graphics

You can also run the following command to clean up the directory:
  make clean


RUNNING INSTRUCTIONS:

usage: graphics <-cube | -points #>;
	- the curses program exits when q is pressed
	- choose either -cube to draw the cube shape or
    - choose -points # to draw random points where # is an integer number of points to draw

To run the program on the SOCS linux machines, it is important to use oclgrind to run the program.

To run the program with oclgrind, run the following command:
  oclgrind ./graphics <-cube | -points #>

NOTES:

On my machine I have OpenCL configured with my NVIDIA GPU, so the program was able to run easily without any slowdown.
However, with the oclgrind simulator (assignment guidelines), the program consumes CPU resources and slows down the program significantly.
This is most likely due to the overhead of the simulator and the fact that the simulator is running on the CPU.

If I am running the program with oclgrind, the performance is notably worse than the serial version of the program provided in the starter code.

