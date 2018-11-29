# SneakyReverseShell
A stealthy remote shell that can hide from processes such as PS and TOP. The
server requires the kernel module to run. The server can support connections
from multiple different clients.

# Build
The code can be built by doing
```
make
```

# Run
Once the kernel module has been inserted the server can be run using the
following command. A port must be specified, if an ip-address is not specified
it will use the local IP
```
server/server.o <port> [ip-address]
```

The client takes the same arguments as the server and can be run by doing:
```
client/client.o <port> [ip-address]
```

