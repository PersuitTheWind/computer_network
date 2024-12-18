#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  uint32_t r = route_prefix & (0xffffffff << (32 - prefix_length));
  auto i = _route_table.find(r);
  if(i == _route_table.end() || it->second.prefix_len <= prefix_length )
      _route_table[r] = {prefix_length , next_hop , interface_num};
}

void Router::route_one_datagram(InternetDatagram &dgram) {
    const uint32_t dst_ip_addr = dgram.header().dst;
    auto end_it  = _route_table.end();
    for(auto i = _route_table.begin() ; i != _route_table.end() ; ++i ){
        uint8_t len = i -> second.prefix_length;
        
        if ( len == 0 || ((i -> first) ^ dst_ip_addr) >> (32 - len）== 0）
            if ( end_it == _route_table.end() || len > end_it > end_it -> second.prefix_length )
                  end_it = it;
                  
    }
    if (end_it != _route_table.end() && dgram.header.ttl > 1){
        dgram.header.ttl--;
        dgram.header.compute_checksum();
        auto it = interface (end_it -> seconde.interface_num);
        it -> send_datagram (dgram , end_it -> second.next_hop.value_or ( Address::from_ipv4_numeric(dst_ip_addr)));
    }
    
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  for (auto &i : _interfaces) {
     auto &q = i -> datagrams_received();
     while( ! q.empty() ){
         route_one_datagram( q.front() );
         q.pop();
     }
  }
}
