/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TORIFY_WS
#define NODEPP_TORIFY_WS

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/ws.h>
#include "http.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace torify { namespace ws {

    inline tcp_torify_t client( const string_t& uri, torify_agent_t* opt=nullptr ){
        auto skt = tcp_torify_t( nullptr, opt ); 
    skt.onSocket.once([=]( ptr_t<tcp_torify_t> self, ws_t cli ){

        http_t raw = cli; cli.set_frame_mask( true );
        if( generator::ws::client( raw, uri )>0 ){

            cli.set_timeout(0); cli.resume(); 
            self->onConnect.resume( );
            self->onConnect.emit(cli); 
            stream::pipe /*--*/ (cli);
            
        }   self->onConnect.skip();

    }); skt.connect( url::hostname(uri), url::port(uri) ); return skt; }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/
