## OS Spring 18 project 3 (Shared Memory & Synchronization)
### Components 
* `coordinator`, responsible for setting up the shared memory block, named semaphores for queueing; 
also compiles all client data and calculates stats at the end.

* `cashier`. There could be up to `MaxNumberOfCashiers` number of cashiers. Each takes several parameters to determine its
service/break time. The dynamically determined total number of cashiers, and the shared memory id should also be passed as
input parameters, in order for the cashier process to access and modify the shared memory correctly. Cashiers are responsible
for enter upcoming client pid and ordered item id into the "database" inside the shared memory.

* `server`. There is only one server process. It randomly selects a food preparation time within the given range, and
keeps the clients waiting for their virtual food. The server also writes the client ordered item price and preparation time
into the database.

* `client`. The maximum total number of clients, and the maximum number of clients in queue (waiting for cashiers) are both
defined in `common.h`. Clients come in and wait in queue for the cashiers and then the server. A client process communicate
its order item to a cashier and the server via the shared memory. After it has got its virtual food and has "eaten", it enters
the total time spent in the "diner" into the database; then it exits.

* Helper functions are defined in `util.h` and implemented in `util.c`; 
common parameters and structs are defined in `common.h`.

### Structure of SHM & semaphores used
The shared memory has a few distinct blocks:
1. An integer to keep track of the number of clients in queue, protected by a named semaphore mutex `client-queue`
2. An integer to keep track of total number of clients that ever entered the diner, also protected by `client-queue`
3. An array of integers to signify the availability of the cashiers (size is of course num_cashiers), protected by
another named semaphore `cashier-queue`
4. A series of `cashier_client_info` structs; each holds the data that needs to be communicated between a cashier and a client,
including: client pid, order item pid, and two unnamed semaphores for synchronization
5. One `server_client_info` struct, holding the data that needs to be communicated between the server and a client, including:
client pid, order item pid, and two unnamed semaphores for synchronization (Note, the server also uses another named semaphore
`server-queue` for queueing clients.
6. A series of `client_db_entry` structs; each uniquely holds the database entry of a client
