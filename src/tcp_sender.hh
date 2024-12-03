#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>

class retransmission_timer{
   private: 
      uint64_t time{0};
      bool Condition{false};
   public:
      retransmission_timer(uint64_t RTO = 0): time(RTO), Condition(false){}
      void reset(uint64_t RTO){
           time = RTO;
           Condition = true;
      }
      bool is_timeout(uint64_t ms_since_last_tick,uint64_t current_RTO_ms_ ){
          time += ms_since_last_tick;
          return Condition && (time >= current_RTO_ms_);
      }
      bool is_open() const{return Condition;}
      void close(){ Condition = false; }
};
   

class TCPSender
{
public:
  /* default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms ),current_RTO_ms_( initial_RTO_ms )
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }

  // Access input stream reader, but const-only (can't read from outside)
  const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  // new variables
  uint64_t current_RTO_ms_;
  Wrap32 Last_sent {Wrap32(0)};
  uint64_t last_ackno {0};
  bool SYN_sent{false};
  uint64_t receiver_window_size{1};
  std::deque<TCPSenderMessage> queue_message {};
  uint64_t consecutive_retransmissions_ {0};
  retransmission_timer Timer{0};
  bool is_finish{false};
  
};
