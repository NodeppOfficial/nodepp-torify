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

    promise_t<https_t,except_t> fetch ( const torify_fetch_t& cfg, const ssl_t* ctx, torify_agent_t* opt=nullptr ) { 
        if( ctx == nullptr ) process::error( "Invalid SSL Context" );
           auto agn = opt == nullptr ? new torify_agent_t() : type::bind( opt ); 
           auto gfc = type::bind( cfg );
           auto ssl = type::bind( ctx );
    return promise_t<https_t,except_t>([=]( function_t<void,https_t> res, function_t<void,except_t> rej ){

        if( !url::is_valid( gfc->url ) ){ rej(except_t("invalid URL")); return; }
        
        url_t    uri = url::parse( gfc->url );
        string_t dir = uri.pathname + uri.search + uri.hash;
        string_t dip = uri.hostname ; gfc->headers["Host"] = dip;
       
        auto client = tls_torify_t ([=]( https_t cli ){ 
            cli.set_timeout( gfc->timeout ); cli.write_header( gfc, dir );

            if( cli.read_header()==0 ){ res( cli ); return; } else { 
                rej(except_t("Could not connect to server")); 
                cli.close(); 
            }
            
        }, &ssl, &agn ); agn->proxy = gfc->proxy;

        client.onError([=]( except_t error ){ rej(error); });
        client.connect( dip, uri.port );

    }); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
