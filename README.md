# C++ WebSocket Chatroom

Simple WebSocket chatroom server in C++ using WebSocket++ and Boost.Asio, plus a browser client.

## Prerequisites (Ubuntu/Debian example)
```bash
sudo apt update
sudo apt install -y build-essential cmake libboost-system-dev libboost-thread-dev
```

WebSocket++ is header-only. If `libwebsocketpp-dev` is not packaged on your distro, install headers manually:
```bash
git clone https://github.com/zaphoyd/websocketpp.git
sudo cp -r websocketpp /usr/local/include/
```

## Build
```bash
mkdir build && cd build
cmake ..
make -j
```

## Run
```bash
./chat_server 9001
```

## Client
Open `client/index.html` in your browser. If server runs on a remote host, edit the client `wsUrl` accordingly.

## Notes
- Set nickname with the UI (or send `/nick yourname` as the first message).
- Add persistence (SQLite) or TLS later if needed.
