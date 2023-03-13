
#include "connection_metadata.h"

namespace ssl=boost::asio::ssl;

void connection_metadata::on_open(client* c, websocketpp::connection_hdl hdl) {
    connection_metadata::m_status = "Open";
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    con->append_header("Origin", "*");
    connection_metadata::m_server = con->get_response_header("Server");
}
 
void connection_metadata::on_fail(client* c, websocketpp::connection_hdl hdl) {
    connection_metadata::m_status = "Failed";
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    connection_metadata::m_server = con->get_response_header("Server");
    connection_metadata::m_error_reason = con->get_ec().message();
    std::cout << "Fail handler" << std::endl;
    std::cout << con->get_state() << std::endl;
    std::cout << con->get_local_close_code() << std::endl;
    std::cout << con->get_local_close_reason() << std::endl;
    std::cout << con->get_remote_close_code() << std::endl;
    std::cout << con->get_remote_close_reason() << std::endl;
    std::cout << con->get_ec() << " - " << con->get_ec().message() << std::endl;
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
        << "> Messages Processed: (" << data.m_messages.size() << ") \n"
        << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason);
 
    std::vector<std::string>::const_iterator it;
    for (it = data.m_messages.begin(); it != data.m_messages.end(); ++it) {
        out << *it << "\n";
    }
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

contextptr connection_metadata::on_tls_init(websocketpp::connection_hdl hdl) {
    contextptr ctx = websocketpp::lib::make_shared<ssl::context>(ssl::context::tlsv1);

    try {
        ctx->set_options(ssl::context::default_workarounds |
                         ssl::context::no_sslv2 |
                         ssl::context::no_sslv3 |
                         ssl::context::single_dh_use);

        ctx->set_verify_mode(ssl::verify_none);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}

bool connection_metadata::verify_certificate(const char* hostname, bool preverified, ssl::verify_context& ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // Retrieve the depth of the current cert in the chain. 0 indicates the
    // actual server cert, upon which we will perform extra validation
    // (specifically, ensuring that the hostname matches. For other certs we
    // will use the 'preverified' flag from Asio, which incorporates a number of
    // non-implementation specific OpenSSL checking, such as the formatting of
    // certs and the trusted status based on the CA certs we imported earlier.
    int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());

    // if we are on the final cert and everything else checks out, ensure that
    // the hostname is present on the list of SANs or the common name (CN).
    if (depth == 0 && preverified) {
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        
        if (connection_metadata::verify_subject_alternative_name(hostname, cert)) {
            return true;
        } else if (connection_metadata::verify_common_name(hostname, cert)) {
            return true;
        } else {
            return false;
        }
    }

    return preverified;
}

/// Verify that one of the subject alternative names matches the given hostname
bool connection_metadata::verify_subject_alternative_name(const char* hostname, X509 * cert) {
    STACK_OF(GENERAL_NAME)* san_names = NULL;
    
    san_names = (STACK_OF(GENERAL_NAME) *) X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (san_names == NULL) {
        return false;
    }
    
    int san_names_count = sk_GENERAL_NAME_num(san_names);
    
    bool result = false;
    
    for (int i = 0; i < san_names_count; i++) {
        const GENERAL_NAME * current_name = sk_GENERAL_NAME_value(san_names, i);
        
        if (current_name->type != GEN_DNS) {
            continue;
        }
        
        char const * dns_name = (char const *) ASN1_STRING_get0_data(current_name->d.dNSName);
        
        // Make sure there isn't an embedded NUL character in the DNS name
        if (ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name)) {
            break;
        }
        // Compare expected hostname with the CN
        result = (strcasecmp(hostname, dns_name) == 0);
    }
    sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
    
    return result;
}

/// Verify that the certificate common name matches the given hostname
bool connection_metadata::verify_common_name(char const * hostname, X509 * cert) {
    // Find the position of the CN field in the Subject field of the certificate
    int common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert), NID_commonName, -1);
    if (common_name_loc < 0) {
        return false;
    }
    
    // Extract the CN field
    X509_NAME_ENTRY * common_name_entry = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
    if (common_name_entry == NULL) {
        return false;
    }
    
    // Convert the CN field to a C string
    ASN1_STRING * common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
    if (common_name_asn1 == NULL) {
        return false;
    }
    
    char const * common_name_str = (char const *) ASN1_STRING_get0_data(common_name_asn1);
    
    // Make sure there isn't an embedded NUL character in the CN
    if (ASN1_STRING_length(common_name_asn1) != strlen(common_name_str)) {
        return false;
    }
    
    // Compare expected hostname with the CN
    return (strcasecmp(hostname, common_name_str) == 0);
}