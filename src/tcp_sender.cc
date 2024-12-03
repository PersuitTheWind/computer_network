#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  uint64_t s = 0;
  if (queue_message.empty()) return 0;
  for(auto i : queue_message)
     s += i.sequence_length();
  return s;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  if (is_finish) return;
  uint64_t seg_size = TCPConfig::MAX_PAYLOAD_SIZE;
  uint64_t w = receiver_window_size ; 
  if (w == 0) w = 1;
  if (seg_size + input_.reader().bytes_popped() > w + last_ackno - 1 ) 
      seg_size = w + last_ackno - 1 - input_.reader().bytes_popped() ;
  if (seg_size > writer().bytes_pushed() - input_.reader().bytes_popped())
      seg_size =  writer().bytes_pushed() - input_.reader().bytes_popped();
  TCPSenderMessage m;
  uint64_t s = seg_size ;
  if( !SYN_sent ){
     m.SYN = true ;
     m.seqno = isn_;
     s++;
  }
  else{
     m.SYN = false;
     m.seqno = Last_sent + static_cast<uint32_t> (1);
  }
  if (writer().is_closed() && input_.reader().bytes_popped() + seg_size == writer().bytes_pushed() && s + input_.reader().bytes_popped() < w + last_ackno - 1 ){
     m.FIN = true;
     is_finish = true;
     s++;
  }
  else m.FIN = false;
  Last_sent = m.seqno + static_cast<uint32_t> (s - 1);
  if(writer().has_error()) m.RST = true;
  if (s == 0) return;
  SYN_sent = true;
  m.payload = reader().peek().substr(0,s);
  input_.reader().pop(s);
  transmit(m);
  queue_message.push_back(m);
  if( !Timer.is_open() ) Timer.reset(0);
  push(transmit);
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage m;
  if(!SYN_sent){
     m.SYN = true;
     m.seqno = isn_;
  }
  else {
     m.SYN = false;
     m.seqno = Last_sent + static_cast<uint32_t>(1);
  }
  if (writer().has_error()) m.RST = true;
  m.payload = "";
  return m;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if (msg.RST) {
     writer().set_error();
     return;
  }
  if (reader().has_error()) return;
  receiver_window_size = msg.window_size;
  if (msg.ackno != std::nullopt){
      uint64_t x = (*msg.ackno).unwrap(isn_,last_ackno) ;
      if (last_ackno != x){
         if(x > input_.reader().bytes_popped() + SYN_sent + is_finish) return;
         last_ackno = x;
         consecutive_retransmissions_ = 0;
         current_RTO_ms_ = initial_RTO_ms_ ;
         while( !queue_message.empty() && queue_message.front().seqno.unwrap(isn_,last_ackno)+ queue_message.front().sequence_length() <= last_ackno ) queue_message.pop_front();
         if (queue_message.empty()) Timer.close();
         else Timer.reset(0);
      }
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if (Timer.is_open())
      if(Timer.is_timeout(ms_since_last_tick,current_RTO_ms_))
        if (!queue_message.empty()){
            transmit(queue_message.front());
            if (receiver_window_size != 0){
                consecutive_retransmissions_ ++;
                current_RTO_ms_ *= 2;
            }
            Timer.reset(0);
        }
}
