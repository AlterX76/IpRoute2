# IpRoute2

This component is an initial C++ implementation to programmatically interact with GNU/Linux kernel interface to execute iproute2 commands.

# Interface

Actually it is possible to create/remove routes and rules and query rt tables.

# Compilation

All the required dependencies are contained in the repo. In addition, it requires libmnl and libnftnl:

```bash
sudo apt install libnftnl-dev
```

To build an application:

```gcc
g++ -I include -I . -lmnl -lnftnl lib/*.c IpRoute2.cpp tests/AddRule.cpp -o AddMyRule.exe
```
