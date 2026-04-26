/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TORIFY_TLS
#define NODEPP_TORIFY_TLS

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/tls.h>

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TOR_AGENT_T
#define NODEPP_TOR_AGENT_T
namespace nodepp { struct torify_agent_t : public agent_t { string_t proxy; };}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class tls_torify_t {
private:

    using NODE_CLB = function_t<void,ssocket_t>;
    enum STATE {
         TLS_STATE_UNKNOWN   = 0b00000000,
         TLS_STATE_USED      = 0b00000001,
         TLS_STATE_CLOSED    = 0b00000010
    };

protected:

    struct NODE {
        int            state= 0;
        torify_agent_t agent;
        ssl_t          ctx  ;
        NODE_CLB       func ;
    };  ptr_t<NODE> obj;

    ptr_t<char> htons( uint16 host_short ) const noexcept {
        ptr_t<char> tmp( 3, '\0' );if( host_short >= 255 ) {
            tmp[0] = type::cast<char>( host_short >> 8 );
            tmp[1] = type::cast<char>( host_short >> 0 );
        } else {
            tmp[1] = type::cast<char>( host_short );
        } return tmp;
    }

public:

    event_t<ssocket_t> onConnect;
    event_t<ssocket_t> onSocket;
    event_t<>          onClose;
    event_t<except_t>  onError;
    event_t<ssocket_t> onOpen;

    /*─······································································─*/

   ~tls_torify_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tls_torify_t( NODE_CLB _func, ssl_t* ssl=nullptr, torify_agent_t* opt=nullptr ) : obj( new NODE() ) {
        obj->agent = (opt==nullptr) ? torify_agent_t() : *opt;
        obj->ctx   = (ssl==nullptr) ? ssl_t() /*----*/ : *ssl; 
        obj->func  = _func; obj->agent.conn_timeout = 0; /*-*/
    }

    tls_torify_t() noexcept : obj( new NODE() ) {}

    /*─······································································─*/

    bool is_closed() const noexcept { return obj->state & STATE::TLS_STATE_CLOSED; }
    void     close() const noexcept { free(); }

    /*─······································································─*/

    void connect( string_t host, int port, NODE_CLB cb=nullptr ) const noexcept {

        if( obj->state & STATE::TLS_STATE_CLOSED )
          { onError.emit( "tcp listener is closed" ); return; } 
        if( obj->state & STATE::TLS_STATE_USED )
          { onError.emit( "tcp listener is used" );   return; } 

        auto prxy  = dns::lookup( obj->agent.proxy );

        if( prxy.null() )
          { onError.emit("Error Proxy not found" ); /*---*/ return; }
        if( obj->ctx.create_client()==-1 )
          { onError.emit("Error Initializing SSL context"); return; }

        ssocket_t sk; obj->state= STATE::TLS_STATE_USED;
        sk.AF      = prxy[0].family;
        sk.SOCK    = SOCK_STREAM;
        sk.IPPROTO = IPPROTO_TCP; 

        if( sk.socket( prxy[0].address, url::port( obj->agent.proxy ) )==-1 ) 
          { onError.emit( "Error while creating TLS" ); return; }
        
        sk.set_sockopt( obj->agent );
        
        sk.ssl = new ssl_t( obj->ctx, sk.get_fd() );
        sk.ssl->set_hostname( host );

        auto self = type::bind(this); process::add([=](){ int c=0;

            while( (c=sk._connect())==-2 ){ return 1; } if(c==-1){
                self->onError.emit( "Error while connecting TLS" );
            return -1; }

            do{ int  len = type::cast<int>( host.size() );
                auto sok = type::cast<socket_t>( sk );

                sok.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x00 }) );
                if( sok.read(2)!=ptr_t<char>({ 0x05, 0x00, 0x00 }) ){
                    self->onError.emit( "Error while Handshaking Sock5" );
                return -1; }

                string_t data = string_t( ptr_t<char>({ 0x05, 0x01, 0x00, 0x03, 0x00 }) )
                              + string_t( ptr_t<char>({ len, 0x00 }) ) + string_t( host )
                              + string_t( htons( port ) ); 
                
                sok.write(data); sok.read( /**/ );

            } while(0);
            
        process::poll( sk, POLL_STATE::WRITE | POLL_STATE::EDGE, [=](){

            cb(sk); self->onSocket.emit(sk); 
            /*---*/ self->obj->func(sk);

            if( sk.is_available() ){ 
                sk.onOpen      .emit(  );
                self->onOpen   .emit(sk); 
                self->onConnect.emit(sk); 
            }

    return -1; }, self->obj->agent.conn_timeout );
    return -1; }); }

    /*─······································································─*/

    void free() const noexcept {
        if( is_closed() ){ return; }
        obj->state = STATE::TLS_STATE_CLOSED; 
        onConnect.clear(); onSocket.clear();
        onError  .clear(); onOpen  .clear();
        onClose  .emit ();
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace torify { namespace tls {

    inline tls_torify_t client( ssl_t* ssl=nullptr, torify_agent_t* opt=nullptr ){
        auto skt = tls_torify_t( nullptr, ssl, opt ); return skt;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

}

/*────────────────────────────────────────────────────────────────────────────*/

#endif

/*────────────────────────────────────────────────────────────────────────────*/