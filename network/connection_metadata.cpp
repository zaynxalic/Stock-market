
#include "connection_metadata.h"

void connection_metadata::on_open(client* c, websocketpp::connection_hdl hdl) {
    connection_metadata::m_status = "Open";
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    connection_metadata::m_server = con->get_response_header("Server");
}
 
void connection_metadata::on_fail(client* c, websocketpp::connection_hdl hdl) {
    connection_metadata::m_status = "Failed";
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    connection_metadata::m_server = con->get_response_header("Server");
    connection_metadata::m_error_reason = con->get_ec().message();
}

void connection_metadata::on_close(client* c, websocketpp::connection_hdl hdl) {
    connection_metadata::m_status = "Closed";
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    std::stringstream s;
    s << "close code: " << con->get_remote_close_code() << " (" 
      << websocketpp::close::status::get_string(con->get_remote_close_code()) 
      << "), close reason: " << con->get_remote_close_reason();
    m_error_reason = s.str();
}

std::ostream& operator<< (std::ostream & out, const connection_metadata& data) {
    out << "> URI: " << data.m_uri << "\n"
        << "> Status: " << data.m_status << "\n"
        << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
        << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason);
    return out;
}

void connection_metadata::record_sent_message(std::string message) {
    m_messages.push_back(">> " + message);
}
 
void connection_metadata::on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
        m_messages.push_back(msg->get_payload());
    } else {
        m_messages.push_back(websocketpp::utility::to_hex(msg->get_payload()));
    }
}

static connection_metadata::context_ptr on_tls_init(websocketpp::connection_hdl hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}