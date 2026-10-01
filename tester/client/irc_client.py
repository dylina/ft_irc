#!/usr/bin/env python3

import socket
import sys
import select

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <port>")
    sys.exit(1)

try:
    port = int(sys.argv[1])
except ValueError:
    print("Port invalide")
    sys.exit(1)

try:
    sock = socket.create_connection(("localhost", port))
except OSError as e:
    print(f"Connection error: {e}")
    sys.exit(1)

print(f"Connected to localhost:{port}")

try:
    while True:
        readable, _, _ = select.select([sock, sys.stdin], [], [])

        if sock in readable:
            data = sock.recv(4096)

            if not data:
                print("\nConnection closed by server.")
                break

            print(data.decode(errors="replace"), end="", flush=True)

        if sys.stdin in readable:
            line = sys.stdin.readline()

            if not line:
                break

            line = line.rstrip("\r\n")

            if line:
                sock.sendall((line + "\r\n").encode())

except KeyboardInterrupt:
    print("\nDisconnected.")

finally:
    sock.close()
