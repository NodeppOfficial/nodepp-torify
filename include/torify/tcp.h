/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_TORIFY_TCP
#define NODEPP_TORIFY_TCP

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/tcp.h>

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

class tcp_torify_t {
private:

    using NODE_CLB = function_t<void,socket_t>;

protected:

    struct NODE {
        char           state = 0;
        torify_agent_t agent;
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

public: tcp_torify_t() noexcept : obj( new NODE() ) {}

    event_t<socket_t> onConnect;
    event_t<socket_t> onSocket;
    event_t<>         onClose;
    event_t<except_t> onError;
    event_t<socket_t> onOpen;

    /*─······································································─*/

   ~tcp_torify_t() noexcept { if( obj.count() > 1 ){ return; } free(); }

    tcp_torify_t( NODE_CLB _func, torify_agent_t* opt=nullptr ) noexcept : obj( new NODE() ) {
        obj->agent = (opt==nullptr) ? torify_agent_t() : *opt; 
        obj->func  = _func; /*------------------------------*/
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
        if( obj->state == 1 ){ return; } if( dns::lookup(obj->agent.proxy).empty() )
          { onError.emit("dns couldn't get ip"); close(); return; }

        auto self = type::bind(this); auto clb = [=](){

            socket_t sk; self->obj->state = 1;
                     sk.SOCK    = SOCK_STREAM;
                     sk.IPPROTO = IPPROTO_TCP;

            if( sk.socket( dns::lookup(
                url::hostname( self->obj->agent.proxy ) ), 
                url::port    ( self->obj->agent.proxy ) )<0 
            ) { self->onError.emit("Error while creating TCP"); 
                self->close(); sk.free(); return -1; 
            }   sk.set_sockopt( self->obj->agent );

            process::poll( sk, POLL_STATE::WRITE, coroutine::add( COROUTINE(){
            int c=0; coBegin

                coWait( (c=sk._connect())==-2 ); if( c<=0 ){
                    self->onError.emit("Error while connecting TCP");
                coEnd; }

                do { int len = type::cast<int>( host.size() );

                    sk.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x00 }) );
                    if( sk.read(2)!=ptr_t<char>({ 0x05, 0x00, 0x00 }) ){
                        self->onError.emit("Error while Handshaking Sock5");
                    coEnd; }

                    string_t data = string_t( ptr_t<char>({ 0x05, 0x01, 0x00, 0x03, 0x00 }) )
                                  + string_t( ptr_t<char>({ len, 0x00 }) ) + string_t( host ) 
                                  + string_t( htons( port ) );
                    
                    sk.write(data); sk.read( /**/ );

                } while(0);

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

namespace torify { namespace tcp {

    inline tcp_torify_t client( torify_agent_t* opt=nullptr ){
        auto skt = tcp_torify_t( nullptr, opt ); return skt;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif