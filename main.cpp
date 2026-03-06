#include <nodepp/nodepp.h>
#include <torify/https.h>
#include <nodepp/url.h>

using namespace nodepp;

void onMain() {
 
    torify_fetch_t args; ssl_t ssl;
    args.timeout = 0; // Disable Fetch timeout
    
    args.url     = "https://www.google.com/";
    args.proxy   = "tcp://localhost:9050";
    args.method  = "GET";

    args.headers = header_t({
        { "Host", url::hostname( args.url ) },
        { "User-Agent", "Torify" }
    });

    torify::https::fetch( args, &ssl )

    .then([]( https_t cli ){
        auto data = stream::await( cli ).value();
        console::log( ">>" , data );
    })

    .fail([]( except_t err ){
        console::log(err);
    });

}
