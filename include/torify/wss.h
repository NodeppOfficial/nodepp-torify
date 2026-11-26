/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TORIFY_WSS
#define NODEPP_TORIFY_WSS

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/wss.h>
#include "https.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace torify { namespace wss {

    inline tls_torify_t client( const string_t& uri, ssl_t* ssl=nullptr, torify_agent_t* opt=nullptr ){
        auto skt = tls_torify_t( nullptr, ssl, opt ); 
    skt.onSocket.once([=]( ptr_t<tls_torify_t> self, wss_t cli ){

        https_t raw = cli; cli.set_frame_mask( true );
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