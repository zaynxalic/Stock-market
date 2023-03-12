#ifndef CONNECTION_METADATA_H
#define CONNECTION_METADATA_H

// #include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
 
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
 

typedef websocketpp::client<websocketpp::config::asio_client> client;
// enabled for tls init handler. Referenced from https://groups.google.com/g/websocketpp/c/SimAUzwZUVM
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;
/*
    This class is used to store the metadata of a connection. It is used to store the connection
*/
class connection_metadata {

    public: 

        typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

        connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri)
            : m_id(id)
            , m_hdl(hdl)
            , m_status("Connecting")
            , m_uri(uri)
            , m_server("N/A")
        {}

        websocketpp::connection_hdl get_hdl() const {
            return m_hdl;
        }

        int get_id() const {
            return m_id;
        }

        std::string get_status() const {
            return m_status;
        }

        std::string get_uri() const {
            return m_uri;
        }

        std::string get_server() const {
            return m_server;
        }

        std::string get_error_reason() const {
            return m_error_reason;
        }

        void on_close(client* c, websocketpp::connection_hdl hdl);
 
        void on_open(client* c,  websocketpp::connection_hdl hdl);

        void on_fail(client* c,  websocketpp::connection_hdl hdl);

        void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);

        void record_sent_message(std::string message);
        
        static contextptr on_tls_init(websocketpp::connection_hdl hdl);

        friend std::ostream& operator<< (std::ostream& out, const connection_metadata& data);

    private:
        int m_id;
        websocketpp::connection_hdl m_hdl;
        std::string m_status;
        std::string m_uri;
        std::string m_server;
        std::string m_error_reason;
        std::vector<std::string> m_messages; // record of all messages sent
};
#endif /* CONNECTION_METADATA_H */
