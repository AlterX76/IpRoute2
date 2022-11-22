# IpRoute2

This component is an initial C++ implementation to programmatically interact with GNU/Linux kernel interface to execute iproute2 commands.

# Interface

Actually it is possible to create/remove routes and rules and query rt tables.

# Compilation

All the required dependencies are contained in the repo. To build an application:

```gcc
g++ -I include -I . lib/*.c IpRoute2.cpp tests/AddRule.cpp -o AddMyRule.exe
```
