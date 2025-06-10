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

    tls_torify_t client( const string_t& uri, const ssl_t* ssl, torify_agent_t* opt=nullptr ){
    tls_torify_t skt   ( [=]( ssocket_t /*unused*/ ){}, ssl, opt );
    skt.onSocket.once  ( [=]( ssocket_t cli ){

        auto hrv = type::cast<https_t>(cli);
        if( !_ws_::client( hrv, uri ) ){ return; }

    process::task::add([=](){ 
        skt.onConnect.once([=]( ssocket_t cli ){ stream::pipe(cli); });
        cli.onDrain  .once([=](){ cli.free(); cli.onData.clear(); });
        cli.set_timeout(0); cli.resume(); skt.onConnect.emit(cli);
    return -1; });

    }); skt.connect( url::hostname(uri), url::port(uri) ); return skt; }

}}}

#endif
