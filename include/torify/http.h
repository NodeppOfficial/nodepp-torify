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

    inline promise_t<http_t,except_t> fetch ( const torify_fetch_t& args, torify_agent_t* opt=nullptr ) { 
           auto agent = type::bind( opt==nullptr ? torify_agent_t():*opt ); 
           auto fetch = type::bind( args ); /*---------------------------*/
    return promise_t<http_t,except_t>([=]( function_t<void,http_t> res, function_t<void,except_t> rej ){

        if( !url::is_valid( fetch->url ) ){ rej(except_t("invalid URL")); return; }
             url_t uri = url::parse( fetch->url );

        if( !fetch->query.empty() ){ uri.search=query::format(fetch->query); }
        string_t dip = uri.hostname ; fetch->headers["Host"] = dip;
        string_t dir = uri.pathname + uri.search + uri.hash;
        agent->proxy = fetch->proxy;

        auto skt = tcp_torify_t ([=]( http_t cli ){ 

            cli.set_timeout( fetch->timeout ); cli.write_header( fetch, dir );
            int c=0; while((c=cli.read_header())==1){ process::next(); }

            if( c==0 ){ res( cli ); return; } cli.close();
            rej(except_t("Could not connect to server"));
            
        }, &agent );

        skt.onError([=]( except_t error ){ rej(error); });
        skt.connect( dip, uri.port );

    }); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
