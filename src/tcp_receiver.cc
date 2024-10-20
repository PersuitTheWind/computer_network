#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if (message.RST) reader().set_error();
  if (!has_received_syn){
     if ( ! message.SYN ) return;
     has_received_syn = true;
     SYN_seqno = message.seqno;
  }
  is_fin = message.FIN;
  uint64_t abs_seqno = message.seqno.unwrap( SYN_seqno , writer().bytes_pushed() );
  uint64_t index = abs_seqno - (!message.SYN) ; 
  reassembler_.insert (index , message.payload , is_fin);
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage Message;
  if (writer().has_error())
     Message.RST = true;
  uint64_t window_size = writer().available_capacity();
  if (window_size > UINT16_MAX ) window_size = UINT16_MAX;
  Message.window_size = window_size;
  if(has_received_syn){
     uint64_t abs_seqno = writer().bytes_pushed() + 1 + writer().is_closed();
     Message.ackno = Wrap32::wrap(abs_seqno ,  SYN_seqno );
  }
  return Message;
}
