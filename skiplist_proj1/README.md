# CSUH 3010 Operating Systems Programming Assignment 1

## Overview
In this programming assignment, I implemented an application that
stores student records in a skiplist data structure. It also provides
an interactive prompt that takes user commands to perform
operations on the record storage.

## File Structure & Components
There are five major components, `structs`, `skiplist`, `myapp`, `io` and `main`.
 * `main.cc` only contains the main function, and is used to initialize the
    application and user prompt.
 * `io.cc` and `io.h` contains user io functions to handle the valid user
    commands, such as `ins` and `range`. The functions also contain various
    input checks (try-catch blocks) to prevent faulty inputs.
 * `myapp.cc` and `myapp.h` contains a class named `MyApp`, which is composed
    of a skiplist object. It behaves like an intermediate step between `io` and
    `skiplist`, in the sense that the `io` module will call on `MyApp` methods,
    in order to perform simple or more involved operations on the skiplist.
 * `skiplist.cc` and `skiplist.h` contains an implementation of the plain old
    skiplist, and supports all necessary operations, such as insert, remove and
    search.
 * `structs.h` contains declarations of the two structs: `Node` and `Record`.
    `Node` is simply the building blocks in the skiplist, and `Record` holds all
    the information related to one particular student ID.

