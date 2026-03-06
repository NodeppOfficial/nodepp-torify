/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TORIFY_HTTPS
#define NODEPP_TORIFY_HTTPS

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/https.h>
#include "tls.h"

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TOR_FETCH_T
#define NODEPP_TOR_FETCH_T
namespace nodepp { struct torify_fetch_t : public fetch_t {
    string_t proxy = "tcp://localhost:9050";
};}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace torify { namespace https {

    inline promise_t<https_t,except_t> fetch ( const torify_fetch_t& args, ssl_t* ssl=nullptr, torify_agent_t* opt=nullptr ) {
           auto agent = type::bind( opt==nullptr ? torify_agent_t():*opt ); 
           auto cert  = type::bind( ssl ); auto fetch = type::bind( args );
    return promise_t<https_t,except_t>([=]( function_t<void,https_t> res, function_t<void,except_t> rej ){

        if( !url::is_valid( fetch->url ) ){ rej(except_t("invalid URL")); return; }
             url_t uri = url::parse( fetch->url );

        if( !fetch->query.empty() ){ uri.search=query::format(fetch->query); }
        string_t dip = uri.hostname ; fetch->headers["Host"] = dip;
        string_t dir = uri.pathname + uri.search + uri.hash;
        agent->proxy = fetch->proxy;

        auto skt = tls_torify_t ([=]( https_t cli ){ 

            cli.set_timeout( fetch->timeout ); cli.write_header( fetch, dir );
            int c=0; while((c=cli.read_header())==1){ process::next(); }

            if( c==0 ){ res( cli ); return; } cli.close();
            rej(except_t("Could not connect to server"));
            
        }, &cert, &agent );

        skt.onError([=]( except_t error ){ rej(error); });
        skt.connect( dip, uri.port );

    }); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
