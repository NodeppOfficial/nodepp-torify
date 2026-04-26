#include <nodepp/nodepp.h>
#include <torify/http.h>
#include <nodepp/url.h>

using namespace nodepp;

void onMain() {
 
    torify_fetch_t args;
    args.timeout = 0; // Disable Fetch timeout
    
    args.proxy   = "tcp://localhost:9050"  ;
    args.url     = "http://www.google.com/";
    args.method  = "GET";

    args.headers = header_t({
        { "Host", url::hostname( args.url ) },
        { "User-Agent", "Torify" }
    });

    torify::http::fetch( args )

    .then([]( http_t cli ){
        auto data = stream::await( cli ).value();
        console::log( ">>" , data );
    })

    .fail([]( except_t err ){
        console::log(err);
    });

}
