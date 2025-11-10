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
namespace nodepp { struct torify_agent_t : public agent_t {
    string_t proxy = "tcp://localhost:9050";
};}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

/*────────────────────────────────────────────────────────────────────────────*/

class tls_torify_t {
private:

    using NODE_CLB = function_t<void,ssocket_t>;

protected:

    struct NODE {
        char           state = 0;
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

public: tls_torify_t() noexcept : obj( new NODE() ) {}

    event_t<ssocket_t> onConnect;
    event_t<ssocket_t> onSocket;
    event_t<>          onClose;
    event_t<except_t>  onError;
    event_t<ssocket_t> onOpen;

    /*─······································································─*/

   ~tls_torify_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tls_torify_t( NODE_CLB _func, ssl_t* ssl=nullptr, torify_agent_t* opt=nullptr ) : obj( new NODE() ) {
        obj->agent = (opt==nullptr) ? torify_agent_t() : *opt; /*--------------*/
        obj->ctx   = (ssl==nullptr) ? ssl_t() /*----*/ : *ssl; obj->func = _func;
    }

    /*─······································································─*/

    void     close() const noexcept { if(obj->state<=0){return;} obj->state=-1; onClose.emit(); }
    bool is_closed() const noexcept { return obj == nullptr ? 1: obj->state<=0; }

    /*─······································································─*/

    void listen( const string_t& host, int port, NODE_CLB cb=nullptr ) const {
         process::error( "servers aren't supported by torify" );
    }

    /*─······································································─*/

    void connect( const string_t& host, int port, NODE_CLB cb=nullptr ) const noexcept {
        if( obj->state == 1 ){ return; } if( obj->ctx.create_client() == -1 )
          { onError.emit("Error Initializing SSL context"); close(); return; }
        if( dns::lookup(obj->agent.proxy).empty() )
          { onError.emit("dns couldn't get ip"); close(); return; }

        auto self = type::bind(this); auto clb = [=](){

            ssocket_t sk; self->obj->state = 1;
                      sk.SOCK    = SOCK_STREAM;
                      sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(
                url::hostname( self->obj->agent.proxy ) ), 
                url::port    ( self->obj->agent.proxy ) )<0 
            ) { self->onError.emit("Error while creating TLS"); 
                self->close(); sk.free(); return -1; 
            }   sk.set_sockopt( self->obj->agent );

            sk.ssl = new ssl_t( self->obj->ctx, sk.get_fd() );
            sk.ssl->set_hostname( host );

            process::poll( sk, POLL_STATE::WRITE, coroutine::add( COROUTINE(){
            int c=0; coBegin

                coWait( (c=sk._connect())==-2 ); if( c<=0 ){
                    self->onError.emit("Error while connecting TLS");
                coEnd; }

                do{ int  len = type::cast<int>( host.size() );
                    auto sok = type::cast<socket_t>( sk );

                    sok.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x00 }) );
                    if( sok.read(2)!=ptr_t<char>({ 0x05, 0x00, 0x00 }) ){
                        self->onError.emit("Error while Handshaking Sock5");
                    coEnd; }

                    string_t data = string_t( ptr_t<char>({ 0x05, 0x01, 0x00, 0x03, 0x00 }) )
                                  + string_t( ptr_t<char>({ len, 0x00 }) ) + string_t( host ) 
                                  + string_t( htons( port ) ); 
                    
                    sok.write(data); sok.read( /**/ );

                } while(0);

                coWait( (c=sk.ssl->_connect())==-2 ); if( c<=0 ){
                    self->onError.emit("Error while handshaking TLS");
                coEnd; }

                sk.onDrain.once([=](){ self->close(); }); cb(sk);
                self->onSocket.emit(sk); self->obj->func(sk);

                if( sk.is_available() ){ 
                    sk.onOpen      .emit(  );
                    self->onOpen   .emit(sk); 
                    self->onConnect.emit(sk); 
                }

            coFinish }));

        return -1; }; process::foop( clb );

    }

    /*─······································································─*/

    void free() const noexcept {
        if( is_closed() ){ return; }close();
        onConnect.clear(); onSocket.clear();
        onError  .clear(); onOpen  .clear();
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

#endif
