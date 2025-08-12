// server/main.cpp
// Build with C++17. Uses websocketpp + Boost.Asio (no TLS).
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <set>
#include <map>
#include <string>
#include <mutex>
#include <sstream>
#include <ctime>
using namespace std;
using websocketpp::connection_hdl;
typedef websocketpp::server<websocketpp::config::asio> server;

class ChatServer {
public:
    ChatServer() {
        m_server.init_asio();

        m_server.set_open_handler([this](connection_hdl hdl) {
            lock_guard<mutex> lock(m_lock);
            m_connections.insert(hdl);
            m_names[hdl] = "Anonymous";
            broadcast_system_message("A user has joined");
        });

        m_server.set_close_handler([this](connection_hdl hdl) {
            lock_guard<mutex> lock(m_lock);
            string name = name_for(hdl);
            m_connections.erase(hdl);
            m_names.erase(hdl);
            broadcast_system_message(name + " has left");
        });

        m_server.set_message_handler([this](connection_hdl hdl, server::message_ptr msg) {
            string payload = msg->get_payload();
            // Simple protocol:
            // If message starts with "/nick ", set nickname
            if (payload.rfind("/nick ", 0) == 0) {
                string newnick = payload.substr(6);
                {
                    lock_guard<mutex> lock(m_lock);
                    m_names[hdl] = sanitize(newnick.empty() ? "Anonymous" : newnick);
                }
                send(hdl, "Nickname set to " + m_names[hdl]);
                broadcast_system_message(m_names[hdl] + " joined as " + m_names[hdl]);
                return;
            }

            // Broadcast normal message with timestamp and nickname
            stringstream ss;
            ss << "[" << timestamp() << "] " << name_for(hdl) << ": " << payload;
            broadcast(ss.str());
        });

        m_server.set_fail_handler([this](connection_hdl hdl) {
            lock_guard<mutex> lock(m_lock);
            // Optional: handle failed connections
            m_connections.erase(hdl);
            m_names.erase(hdl);
        });
    }

    void run(uint16_t port) {
        m_server.listen(port);
        m_server.start_accept();
        cout << "Chat server listening on ws://0.0.0.0:" << port << "\n";
        m_server.run();
    }

private:
    server m_server;
    set<connection_hdl, owner_less<connection_hdl>> m_connections;
    map<connection_hdl, string, owner_less<connection_hdl>> m_names;
    mutex m_lock;

    string name_for(connection_hdl hdl) {
        auto it = m_names.find(hdl);
        if (it != m_names.end()) return it->second;
        return string("Anonymous");
    }

    void send(connection_hdl hdl, const string& msg) {
        try {
            m_server.send(hdl, msg, websocketpp::frame::opcode::text);
        } catch (const websocketpp::exception& e) {
            cerr << "Send failed: " << e.what() << "\n";
        }
    }

    void broadcast(const string& msg) {
        lock_guard<mutex> lock(m_lock);
        for (auto it : m_connections) {
            try {
                m_server.send(it, msg, websocketpp::frame::opcode::text);
            } catch (const websocketpp::exception& e) {
                cerr << "Broadcast failed: " << e.what() << "\n";
            }
        }
    }

    void broadcast_system_message(const string& text) {
        stringstream ss;
        ss << "[" << timestamp() << "] " << "[system] " << text;
        broadcast(ss.str());
    }

    string timestamp() {
        time_t t = time(nullptr);
        tm tm;
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        return string(buf);
    }

    string sanitize(const string& s) {
        string out;
        for (char c : s) {
            if (c == '\n' || c == '\r') continue;
            out.push_back(c);
        }
        return out;
    }
};

int main(int argc, char* argv[]) {
    uint16_t port = 9001;
    if (argc > 1) {
        port = static_cast<uint16_t>(stoi(argv[1]));
    }

    try {
        ChatServer server;
        server.run(port);
    } catch (const exception& e) {
        cerr << "Fatal: " << e.what() << "\n";
    }
    return 0;
}
