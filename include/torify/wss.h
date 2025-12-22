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
    tls_torify_t skt   ( nullptr, ssl, opt );
    skt.onSocket.once  ( [=]( ssocket_t cli ){

        auto hrv = type::cast<https_t>(cli);
        if(!generator::ws::client( hrv, uri ) )
          { skt.onConnect.skip(); return; }   

        process::foop([=](){ 
            cli.set_timeout(0); cli.resume(); 
            skt.onConnect.resume( );
            skt.onConnect.emit(cli); 
        return -1; });

    }); skt.connect( url::hostname(uri), url::port(uri) ); return skt; }

}}}

#endif
