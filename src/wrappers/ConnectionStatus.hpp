#pragma once

#include <EventData.h>

#include <string>
#include <unordered_map>

namespace dxfcs {

/**
 * Wrapper over the dxf_connection_status_t enum
 */
class ConnectionStatus {
    dxf_connection_status_t status_;
    std::string string_;

    ConnectionStatus(dxf_connection_status_t status, std::string string)
        : status_{status}, string_{std::move(string)} {}

  public:
    static const ConnectionStatus NOT_CONNECTED;
    static const ConnectionStatus CONNECTED;
    static const ConnectionStatus LOGIN_REQUIRED;
    static const ConnectionStatus AUTHORIZED;

    const static std::unordered_map<dxf_connection_status_t, ConnectionStatus> ALL;

    static ConnectionStatus get(dxf_connection_status_t status) { return ALL.at(status); }

    dxf_connection_status_t getStatus() const { return status_; }

    const std::string &toString() const { return string_; }

    template <class Ostream> friend Ostream &&operator<<(Ostream &&os, const ConnectionStatus &value) {
        return std::forward<Ostream>(os) << value.toString();
    }
};

const ConnectionStatus ConnectionStatus::NOT_CONNECTED{dxf_cs_not_connected, "Not connected"};
const ConnectionStatus ConnectionStatus::CONNECTED{dxf_cs_connected, "Connected"};
const ConnectionStatus ConnectionStatus::LOGIN_REQUIRED{dxf_cs_login_required, "Login required"};
const ConnectionStatus ConnectionStatus::AUTHORIZED{dxf_cs_authorized, "Authorized"};

const std::unordered_map<dxf_connection_status_t, ConnectionStatus> ConnectionStatus::ALL{
    {NOT_CONNECTED.getStatus(), NOT_CONNECTED},
    {CONNECTED.getStatus(), CONNECTED},
    {LOGIN_REQUIRED.getStatus(), LOGIN_REQUIRED},
    {AUTHORIZED.getStatus(), AUTHORIZED}};
} // namespace dxfcs