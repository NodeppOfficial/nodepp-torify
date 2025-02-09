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
protected:

    struct NODE {
        int                       state = 0;
        torify_agent_t            agent;
        poll_t                    poll ;
        function_t<void,socket_t> func ;
    };  ptr_t<NODE> obj;

    ptr_t<char> htons( uint16 host_short ) const noexcept {
        ptr_t<char> tmp( 3, '\0' ); if( host_short >= 255 ) { 
            tmp[0] = (char)( host_short >> 8 );
            tmp[1] = (char)( host_short >> 0 );
        } else { 
            tmp[1] = (char)( host_short );
        } return tmp;
    }
    
public: tcp_torify_t() noexcept : obj( new NODE() ) {}

    event_t<socket_t> onConnect;
    event_t<socket_t> onSocket;
    event_t<>         onClose;
    event_t<except_t> onError;
    event_t<socket_t> onOpen;
    
    /*─······································································─*/
    
    tcp_torify_t( decltype(NODE::func) _func, torify_agent_t* opt=nullptr ) noexcept : obj( new NODE() )
         { obj->func=_func; obj->agent=opt==nullptr?torify_agent_t():*opt; }
    
    /*─······································································─*/
    
    void     close() const noexcept { if( obj->state<=0 ){ return; } obj->state=-1; onClose.emit(); }
    
    bool is_closed() const noexcept { return obj == nullptr ? 1 : obj->state <= 0; }
    
    /*─······································································─*/

    void listen( const string_t& host, int port, decltype(NODE::func) cb ) const {
         process::error( "servers aren't supported by torify" );
    }

    void listen( const string_t& host, int port ) const noexcept { 
         listen( host, port, []( socket_t ){} ); 
    }

    /*─······································································─*/

    void connect( const string_t& host, int port, decltype(NODE::func) cb ) const noexcept {
        if( obj->state == 1 ){ return; } if( dns::lookup(host).empty() )
          { _EERROR(onError,"dns couldn't get ip"); close(); return; }
        
        auto self = type::bind( this ); obj->state = 1;

        socket_t sk;
                 sk.SOCK    = SOCK_STREAM; 
                 sk.IPPROTO = IPPROTO_TCP;
                 sk.socket( dns::lookup(
                       url::hostname( obj->agent.proxy ) 
                    ), url::port( obj->agent.proxy )
                ); sk.set_sockopt( self->obj->agent );

        process::poll::add([=](){
            if( self->is_closed() ){ return -1; }
        coStart

            while( sk._connect() == -2 ){ coNext; } 
            if   ( sk._connect()  <  0 ){ 
                _EERROR(self->onError,"Error while connecting TCP"); 
            coEnd; }

            if( self->obj->poll.push_write(sk.get_fd())==0 )
              { sk.free(); } while( self->obj->poll.emit()==0 ){ 
                   if( process::now() > sk.get_send_timeout() )
                     { coEnd; } coNext; }

            do { int len = type::cast<int>( host.size() );

                sk.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x00 }) );
                if( sk.read(2)!=ptr_t<char>({ 0x05, 0x00, 0x00 }) ){ 
                    _EERROR(self->onError,"Error while Handshaking Sock5"); 
                coEnd; } 

                sk.write( ptr_t<char>({ 0x05, 0x01, 0x00, 0x03, 0x00 }) );
                sk.write( ptr_t<char>({ len, 0x00 }) ); sk.write( host );
                sk.write( htons( port ) ); sk.read();

            } while(0); cb( sk );
            
            sk.onClose.once([=](){ self->close(); }); 
            self->onSocket.emit(sk); sk.onOpen.emit(); 
            self->onOpen.emit(sk); self->obj->func(sk);
            
        coStop
        });

    }

    void connect( const string_t& host, int port ) const noexcept { 
         connect( host, port, [=]( socket_t ){} ); 
    }

};

/*────────────────────────────────────────────────────────────────────────────*/

namespace torify { namespace tcp {

    tcp_torify_t client( const tcp_torify_t& client ){ client.onOpen.once([=]( socket_t cli ){
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

    tcp_torify_t client( torify_agent_t* opt=nullptr ){
        auto client = tcp_torify_t( [=]( socket_t /*unused*/ ){}, opt );
        tcp::client( client ); return client; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

}

#endif
