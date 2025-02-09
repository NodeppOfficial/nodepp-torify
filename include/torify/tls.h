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
protected:

    struct NODE {
        int                        state = 0;
        torify_agent_t             agent;
        ssl_t                      ctx  ; 
        poll_t                     poll ;
        function_t<void,ssocket_t> func ;
    };  ptr_t<NODE> obj;

    ptr_t<char> htons( uint16 host_short ) const noexcept {
        if( host_short >= 255 ) { 
            ptr_t<char> tmp( 3, '\0' );
            tmp[0] = (char)( host_short >> 8 );
            tmp[1] = (char)( host_short >> 0 );
            return tmp; 
        } else { 
            ptr_t<char> tmp( 3, '\0' );
            tmp[1] = (char)( host_short );
            return tmp;
        } return nullptr;
    }
    
public: tls_torify_t() noexcept : obj( new NODE() ) {}

    event_t<ssocket_t> onConnect;
    event_t<ssocket_t> onSocket;
    event_t<>          onClose;
    event_t<except_t>  onError;
    event_t<ssocket_t> onOpen;
    
    /*─······································································─*/

    tls_torify_t( decltype(NODE::func) _func, const ssl_t* xtc, torify_agent_t* opt=nullptr )
    : obj( new NODE() ){ 
    if( xtc == nullptr ) process::error("Invalid SSL Contenx");
        obj->agent = opt==nullptr ? torify_agent_t():*opt; 
        obj->ctx   = xtc==nullptr ? ssl_t():  *xtc; 
        obj->func  = _func;
    }

    /*─······································································─*/
    
    void     close() const noexcept { if( obj->state<=0 ){ return; } obj->state=-1; onClose.emit(); }

    bool is_closed() const noexcept { return obj == nullptr ? 1 : obj->state <= 0; }
    
    /*─······································································─*/

    void listen( const string_t& host, int port, decltype(NODE::func) cb ) const {
         process::error( "servers aren't supported by torify" );
    }

    void listen( const string_t& host, int port ) const noexcept { 
         listen( host, port, []( ssocket_t ){} ); 
    }
    
    /*─······································································─*/

    void connect( const string_t& host, int port, decltype(NODE::func) cb  ) const noexcept {
        if( obj->state == 1 ){ return; } if( obj->ctx.create_client() == -1 )
          { _EERROR(onError,"Error Initializing SSL context"); close(); return; }
        if( dns::lookup(host).empty() )
          { _EERROR(onError,"dns couldn't get ip"); close(); return; }

        auto self = type::bind( this ); obj->state = 1;

        ssocket_t sk; 
                  sk.SOCK    = SOCK_STREAM;
                  sk.IPPROTO = IPPROTO_TCP;
                  sk.socket( dns::lookup(
                       url::hostname( obj->agent.proxy ) 
                    ), url::port( obj->agent.proxy )
                  ); sk.set_sockopt( self->obj->agent );

        sk.ssl = new ssl_t( obj->ctx, sk.get_fd() ); 
        sk.ssl->set_hostname( host );

        process::poll::add([=](){
            if( self->is_closed() ){ return -1; }
        coStart

            while( sk._connect() == -2 ){ coNext; } 
            if   ( sk._connect()  <  0 ){ 
                _EERROR(self->onError,"Error while connecting TLS"); 
            coEnd; }

            if( self->obj->poll.push_write(sk.get_fd())==0 )
              { sk.free(); } while( self->obj->poll.emit()==0 ){ 
                   if( process::now() > sk.get_send_timeout() )
                     { coEnd; } coNext; }

            do { int  len = type::cast<int>( host.size() );
                 auto sok = (socket_t)sk;

                sok.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x00 }) );
                if( sok.read(2)!=ptr_t<char>({ 0x05, 0x00, 0x00 }) ){ 
                    _EERROR(self->onError,"Error while Handshaking Sock5"); 
                coEnd; } 

                sok.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x03, 0x00 }) );
                sok.write( ptr_t<char>({ len, 0x00 }) ); sok.write( host );
                sok.write( htons( port ) ); sok.read();

            } while(0);

            while( sk.ssl->_connect() == -2 ){ coNext; }
            if   ( sk.ssl->_connect() <=  0 ){ 
                _EERROR(self->onError,"Error while handshaking TLS");
            coEnd; } cb( sk );
            
            sk.onClose.once([=](){ self->close(); }); 
            self->onSocket.emit(sk); sk.onOpen.emit(); 
            self->onOpen.emit(sk); self->obj->func(sk);

        coStop
        });

    }

    void connect( const string_t& host, int port ) const noexcept { 
         connect( host, port, []( ssocket_t ){} ); 
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace torify { namespace tls {

    tls_torify_t client( const tls_torify_t& client ){ client.onOpen.once([=]( ssocket_t cli ){
        cli.onDrain.once([=](){ cli.free(); cli.onData.clear(); });
        ptr_t<_file_::read> _read = new _file_::read;

        process::poll::add([=](){
            if(!cli.is_available() )    { cli.close(); return -1; }
            if((*_read)(&cli)==1 )      { return 1; } 
            if(  _read->state<=0 )      { return 1; }
            cli.onData.emit(_read->data); return 1;
        });

    }); return client; }

    /*─······································································─*/

    tls_torify_t client( const ssl_t* ctx, torify_agent_t* opt=nullptr ){
        auto client = tls_torify_t( [=]( ssocket_t /*unused*/ ){}, ctx, opt );
        tls::client( client ); return client; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif
