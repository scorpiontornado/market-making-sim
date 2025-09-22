# Plan
## Short-term
- Each trader has a unique ID (int) and we operate on those.
    - ID is the index of the trader for now.
- We make a Name->ID map
- In registration, do we have to enforce unique names?
- Implement matching engine
- Move core logic (order book, matching, traders etc.) to a library, `exchange/`

### Option 1 - per-client sessions
Welcome to our platform! Enter your name:
> Hugo
Make trades in the following format: BUY <SHARE> <QUANTITY> @ <PRICE>
> BUY ASX:DHHF 100 @ 30

### Option 2 - provide username with each command
> BUY Alice ASX:DHHF 100 @ 50

## Long-term
- Client/server, cmake, testing
- Multithreaded server
- Networking (TCP sockets, or async)
- When making it multithreaded, will have to make vectors private rather than global, & use unique/sharedptr (or just use a mutex)

### Stages:
1. Single-process, TUI Exchange
    1.	Define Order and Trader structs/classes.
    2.	Write parser to read an order from user input.
    3.	Store orders in separate buy/sell containers.
    4.	Implement matching engine for one buy and one sell.
    5.	Expand to handle multiple orders and partial fills.
    6.	Print order book + trader states after each command.
    7.	Add unit tests for the matching function.

2. Multi-client via Pipes / StdIn-Out
   1. Make server/main.cpp, handle connections & spawning threads etc

3. Networking (Sockets)
