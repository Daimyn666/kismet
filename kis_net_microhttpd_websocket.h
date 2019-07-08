/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __KIS_NET_MICROHTTPD_WEBSOCKET_H__
#define __KIS_NET_MICROHTTPD_WEBSOCKET_H__ 

#include "config.h"
#include "kis_net_microhttpd.h"
#include "kis_net_microhttpd_handlers.h"

// Websocket handler to handle an upgrade and handshake on a ws:// URI and then
// create a pollable object
class Kis_Net_Httpd_Websocket_Handler : public Kis_Net_Httpd_Handler {
public:
    Kis_Net_Httpd_Websocket_Handler() : Kis_Net_Httpd_Handler() { }
    virtual ~Kis_Net_Httpd_Websocket_Handler();

    virtual int Httpd_HandleGetRequest(Kis_Net_Httpd *httpd,
            Kis_Net_Httpd_Connection *connection,
            const char *url, const char *method, const char *upload_data,
            size_t *upload_data_size) override;

    // Can this handler process this request?
    virtual bool Httpd_VerifyPath(const char *path, const char *method) override = 0;

protected:
    // Perform a websocket upgrade (returning true) to the connection, or fail to upgrade,
    // push the error to the connection, and return false
    bool Httpd_Websocket_Upgrade(Kis_Net_Httpd_Connection *connection);

    std::vector<std::string> ws_protocols;
};

#endif /* ifndef KIS_NET_MICROHTTPD_WEBSOCKET_H */
