*This project has been created as part of the 42 curriculum by ypellegr, dgorceac, mleschev.*

# ft_irc

## Description

**ft_irc** is an IRC (Internet Relay Chat) server written in **C++98**.

IRC is a text-based protocol for real-time messaging: clients connect to a server, choose a nickname, join channels (group conversations) and exchange public or private messages. The goal of this project is to implement such a server from scratch, compatible with a real IRC client, without relying on any external library.

### Overview

- **Single process, no fork, no threads**: all clients are handled at the same time by **one `poll()` loop**.
- **Non-blocking I/O**: sockets are set to non-blocking mode, and every read and write goes through `poll()` (`POLLIN` to read, `POLLOUT` to write), so one slow client never freezes the others.
- **Buffered input**: TCP is a byte stream, so a command can arrive in several pieces. Data is accumulated per client and only parsed once a full line (ending in `\n` or `\r\n`) is received.
- **Buffered output**: replies are queued per client and sent when the socket is ready to be written.
- **Reference client**: [irssi](https://irssi.org/).

### Features

**Authentication / registration**

| Command | Description |
|---|---|
| `PASS <password>` | Server password, required before registering |
| `NICK <nickname>` | Choose or change your nickname |
| `USER <username> <mode> <unused> :<realname>` | Set your username |

Once `PASS`, `NICK` and `USER` are valid, the server sends the welcome replies (`001` to `004`). Before that, any other command is answered with `451 You have not registered`.

**Messaging and channels**

| Command | Description |
|---|---|
| `JOIN <#channel> [key]` | Join a channel (created if it does not exist) |
| `PART <#channel> [:reason]` | Leave a channel |
| `PRIVMSG <target> :<message>` | Send a message to a channel or a user |
| `TOPIC <#channel> [:topic]` | View or change the channel topic |
| `QUIT [:reason]` | Disconnect from the server |
| `PING <token>` | Connection check, answered with `PONG` |

**Channel operator commands**

| Command | Description |
|---|---|
| `KICK <#channel> <nick> [:reason]` | Eject a user from the channel |
| `INVITE <nick> <#channel>` | Invite a user to the channel |
| `TOPIC` | Change the topic (restricted to operators with mode `+t`) |
| `MODE <#channel> <modes> [params]` | Change the channel modes |

**Channel modes**

| Mode | Effect |
|---|---|
| `i` | Invite-only channel |
| `t` | Only operators can change the topic |
| `k` | Channel key (password) |
| `o` | Give / take channel operator privilege |
| `l` | Limit the number of users |

## Instructions

### Requirements

- A C++ compiler supporting C++98 (`c++`)
- `make`
- Linux or macOS

### Compilation

```bash
make        # build ./ircserv
make clean  # remove object files
make fclean # remove object files and the executable
make re     # rebuild from scratch
```

The code is compiled with `-Wall -Wextra -Werror -std=c++98`.

### Execution

```bash
./ircserv <port> <password>
```

- `port`: the port the server listens on (for example `6667`)
- `password`: the password clients must give with `PASS`

Example:

```bash
./ircserv 6667 mypassword
```

### Connecting with irssi

```bash
irssi
/connect 127.0.0.1 6667 mypassword alice
/join #general
/msg #general Hello everyone!
```

### Connecting with nc

`nc` lets you type raw IRC commands. Use `-C` to send `\r\n` line endings (plain `\n` is also accepted).

```bash
nc -C 127.0.0.1 6667
PASS mypassword
NICK alice
USER alice 0 * :Alice Smith
JOIN #general
PRIVMSG #general :Hello everyone!
```

### Partial data test

The server must handle commands received in several pieces. In `nc`, press `Ctrl+D` to send what has been typed so far without a newline:

```bash
nc -C 127.0.0.1 6667
com^Dman^Dd
```

The server waits for the end of the line before processing the command.

## Resources

### Documentation

- [RFC 1459 – Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [RFC 2812 – Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
- [Modern IRC Client Protocol (ircdocs)](https://modern.ircdocs.horse/) – up-to-date description of commands and numeric replies
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) – sockets, `poll()`, non-blocking I/O
- [irssi documentation](https://irssi.org/documentation/)
- Linux manual pages: `socket(2)`, `bind(2)`, `listen(2)`, `accept(2)`, `poll(2)`, `recv(2)`, `send(2)`, `fcntl(2)`, `setsockopt(2)`

### Use of AI

AI was used as a learning and review assistant. Every piece of code it suggested was read, understood, tested and adapted by the team before being kept.

  - Explanations of the IRC protocol, sockets, `poll()` and the registration flow.
  - Help writing and fixing the authentication handlers, and the line-ending handling (`\n` / `\r\n`).
  - Writing test scenarios with `nc` to check every authentication case.

