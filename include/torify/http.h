/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TORIFY_HTTP
#define NODEPP_TORIFY_HTTP

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/http.h>
#include "tcp.h"

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TOR_FETCH_T
#define NODEPP_TOR_FETCH_T
namespace nodepp { struct torify_fetch_t : public fetch_t {
    string_t proxy = "tcp://localhost:9050";
};}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace torify { namespace http {

    promise_t<http_t,except_t> fetch ( const torify_fetch_t& cfg, torify_agent_t* opt=nullptr ) { 
           auto agn = opt == nullptr ? new torify_agent_t() : type::bind( opt ); 
           auto gfc = type::bind( cfg ); 
    return promise_t<http_t,except_t>([=]( function_t<void,http_t> res, function_t<void,except_t> rej ){

        if( !url::is_valid( gfc->url ) ){ rej(except_t("invalid URL")); return; }
        
        url_t    uri = url::parse( gfc->url );
        string_t dir = uri.pathname + uri.search + uri.hash;
        string_t dip = uri.hostname ; gfc->headers["Host"] = dip;

        auto client = tcp_torify_t ([=]( http_t cli ){ 
            cli.set_timeout( gfc->timeout ); cli.write_header( gfc, dir );

            if( cli.read_header()==0 ){ res( cli ); return; } else { 
                rej(except_t("Could not connect to server")); 
                cli.close(); 
            }
            
        }, &agn ); agn->proxy = gfc->proxy;

        client.onError([=]( except_t error ){ rej(error); });
        client.connect( dip, uri.port );

    }); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
