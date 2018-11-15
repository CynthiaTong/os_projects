# OS Spring 18 project 2: multi-process sorting

## Overview and Code Modules
In this project, I implemented a multi-process sorting system, which has the following code modules:

1. `root.c` contains the code needed to compile and run the `mysorter` executable. It takes the command line arguments, fork and exec `merge_split` to carry out the sorting task. It also redirects the stdout of its child if an output file (-o) is specified. The root is also responsible for collecting and displaying timing and signal data of the current run of the system. In particular, it displays the actual and expected numbers of `USR1`, `USR2` and `ALRM` signal it receives from the sorters, and the total turnaround time of this sorting execution in milliseconds. It also executes the `cat` system program to display the timing data files inside the `timing` directory.

2. `merge_split.c` contains code to be executed by merger/splitter nodes. As the name suggests, it is in charge of two main tasks: split and merge. Split means splitting the sorting ranges for its left and right child; merge means merging the records data coming from the pipe of each child, and sending the merged data back up to the current node's parent (through another pipe). Noticably, the first merger/splitter node does not have a pipe from its parent (the root process), it instead writes directly to stdout. Each merger/splitter writes to its own timing data file inside the `timing` directory.

3. `sort.c` contains code for the sorter (leaf) nodes. the Sorters fork and exec one of the three sorting executables, `SH`, `QS` and `BS`. The choice of which sorting to use is made based on the index of the sorter - sorter#0 always uses `SH`, #1 uses `QS`, #2 uses `BS`, #3 uses `SH` again, etc. Each sorter writes to its own timing data file inside the `timing` directory.

4. `shell_sort.c`, `quick_sort.c`, `bubble_sort.c`, each contains code for the respective sorting algorithm, and can be invoked from the command line.


## Note:
A few implementation details to notice:

* The sorting criteria are not cascaded, meaning: if we are sorting based on one attribute, say 1(first name),
then if two records have the same first name, then the order they are sorted is arbitrary, since we do not sort
again based on any secondary condition.
