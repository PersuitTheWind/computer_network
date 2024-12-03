#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  uint32_t next_ip = next_hop.ipv4_numeric();
  auto i = addr_cache.find(next_ip);
  if(i != addr_cache.end())
    transmit(make_frame((*i).second.first, EthernetHeader::TYPE_IPv4, serialize(dgram)));
  else {
    auto i1 = req_time.find(next_ip);
    if(i1 == req_time.end() or Time - (*i1).second > 5000){
      ARPMessage m;
      m.sender_ip_address = ip_address_.ipv4_numeric();
      m.sender_ethernet_address = ethernet_address_;
      m.target_ip_address = next_ip;
      m.opcode = ARPMessage::OPCODE_REQUEST;
      transmit(make_frame(ETHERNET_BROADCAST, EthernetHeader::TYPE_ARP,serialize(m)));
      if (i1 == req_time.end()) req_time.emplace(next_ip,Time);
      else (*i1).second = Time;
    }
    queueing_dgrams.emplace_back( make_pair(next_ip,dgram) );
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  const auto &head = frame.header;
  if (head.dst == ethernet_address_ || head.dst == ETHERNET_BROADCAST) {
     if(head.type == EthernetHeader::TYPE_IPv4){
         InternetDatagram dgram;
         if (parse(dgram,frame.payload))
             datagrams_received_.emplace(dgram);
     }
     else if(head.type == EthernetHeader::TYPE_ARP){
         ARPMessage m ;
         if (parse(m,frame.payload)){
             auto i = addr_cache.find(m.sender_ip_address);
             if (i != addr_cache.end()){
                (*i).second.first = m.sender_ethernet_address;
                (*i).second.second = Time;
             }
             else 
                addr_cache.emplace(m.sender_ip_address, make_pair(m.sender_ethernet_address, Time));
             auto i1 = req_time.find(m.sender_ip_address);
             if(i1 != req_time.end()) req_time.erase(i1);
             uint32_t ip0 = m.sender_ip_address;
             auto it = queueing_dgrams.begin();
             while(it != queueing_dgrams.end()){
                if ((*it).first == ip0){
                    transmit(make_frame(addr_cache[ip0].first, EthernetHeader::TYPE_IPv4, serialize((*it).second)));
                    it = queueing_dgrams.erase(it);
                }
                else ++it;
             }
             if (m.opcode == ARPMessage::OPCODE_REQUEST && m.target_ip_address == ip_address_.ipv4_numeric()){
                ARPMessage m_reply;
                m_reply.sender_ethernet_address = ethernet_address_;
                m_reply.sender_ip_address = ip_address_.ipv4_numeric();
                m_reply.target_ethernet_address = m.sender_ethernet_address;
                m_reply.target_ip_address = m.sender_ip_address;
                m_reply.opcode = ARPMessage::OPCODE_REPLY ;
                transmit(make_frame(m.sender_ethernet_address, EthernetHeader::TYPE_ARP,serialize(m_reply)));
             }
          }
      }
   }     
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  Time += ms_since_last_tick;
  auto i = addr_cache.begin();
  while(i != addr_cache.end())
       if (Time - (*i).second.second > 30000)
            i = addr_cache.erase(i);
       else ++i;
}

EthernetFrame NetworkInterface::make_frame(const EthernetAddress& dst, uint16_t type,vector<string> payload){
      EthernetFrame f;
      f.header.src = ethernet_address_;
      f.header.dst = dst;
      f.header.type = type;
      f.payload = std::move(payload);
      return f;
}
