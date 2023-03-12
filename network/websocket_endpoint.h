#ifndef WEBSOCKET_ENDPOINT_H
#define WEBSOCKET_ENDPOINT_H
#include "connection_metadata.h"

class websocket_endpoint {

// m_endpoint is the client object.
public:

    websocket_endpoint () : m_next_id(0) {
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
        m_endpoint.init_asio();
        m_endpoint.start_perpetual();
        m_thread.reset(new websocketpp::lib::thread(&client::run, &m_endpoint));
    }
    
    int connect(std::string const & uri);
 
    connection_metadata::ptr get_metadata(int id) const;

    void send (int id, std::string message);

    ~websocket_endpoint() {
        m_endpoint.stop_perpetual();
        
        for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
            if (it->second->get_status() != "Open") {
                // Only close open connections
                continue;
            }
            
            std::cout << "> Closing connection " << it->second->get_id() << std::endl;
            
            websocketpp::lib::error_code ec;
            m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
            if (ec) {
                std::cout << "> Error closing connection " << it->second->get_id() << ": "  
                        << ec.message() << std::endl;
            }
        }
        
        m_thread->join();
    }
    
private:
    // create the connect list <connection_id, with the connection list>
    typedef std::map<int,connection_metadata::ptr> con_list;
    typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;
    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
 
    con_list m_connection_list;
    int m_next_id;
};

#endif /* WEBSOCKET_ENDPOINT_H */