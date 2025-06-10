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

    tcp_torify_t client( const string_t& uri, torify_agent_t* opt=nullptr ){ 
    tcp_torify_t skt   ( [=]( socket_t /*unused*/ ){}, opt );
    skt.onSocket.once  ( [=]( socket_t cli ){

        auto hrv = type::cast<http_t> (cli);
        if( !_ws_::client( hrv, uri ) ){ return; }

    process::task::add([=](){
        cli.onDrain  .once([=](){ cli.free(); });
        skt.onConnect.once([=]( ws_t ctx ){ stream::pipe(ctx); });
        cli.set_timeout(0); cli.resume(); skt.onConnect.emit(cli);
    return -1; });

    }); skt.connect( url::hostname(uri), url::port(uri) ); return skt; }

}}}

#endif