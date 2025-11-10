# NodePP Tor Client: Basic Onion Routing for NodePP

This project provides a fundamental Tor client implementation for the NodePP framework. Tor (The Onion Router) is a network that helps you defend against traffic analysis, a form of network surveillance that threatens personal freedom and privacy, confidential business activities and relationships, and state security. This library aims to allow NodePP applications to route their network traffic through the Tor network for enhanced anonymity and privacy.

## Key Features

- **Basic Tor Connection:** Establishes a connection to the Tor network.
- **SOCKS5 Proxy Support:** Implements a SOCKS5 proxy client to forward traffic through Tor.
- **Simple API:** Offers a straightforward interface to connect and send data over Tor within NodePP.
- **Asynchronous Operations:** Designed to integrate seamlessly with NodePP's asynchronous event loop.

## Usage 
```cpp
#include <nodepp/nodepp.h>
#include <nodepp/url.h> 
#include <torify/http.h>

using namespace nodepp;

void onMain() {

    tor_fetch_t args;
    args.timeout = 0; // Disable Fetch timeout
    args.method  = "GET";
    args.proxy   = "tcp://localhost:9050";
    args.url     = "http://check.torproject.org/";
    args.headers = header_t({
        { "Host", url::hostname( args.url ) }
    });

    tor::http::fetch( args )

    .then([]( http_t cli ){
        stream::pipe( cli, fs::std_output() );
    })

    .fail([]( except_t err ){
        console::log(err);
    });

}
```

## Build & Run
```bash
🪟: g++ -o main main.cpp -I ./include -lws2_32 ; ./main
🐧: g++ -o main main.cpp -I ./include ; ./main
```

## License

**Nodepp** is distributed under the MIT License. See the LICENSE file for more details.
