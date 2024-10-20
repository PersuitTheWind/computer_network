#include "wrapping_integers.hh"

using namespace std;

const uint64_t WRAP_LENGTH = 1ul << 32; 

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  uint32_t nn = n;
  return zero_point + nn;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint32_t check_seq = wrap(checkpoint,zero_point).raw_value_;
  int32_t distance = raw_value_ - check_seq; 
  int64_t abs_seqno = checkpoint + distance ;
  if  ( abs_seqno < 0 )
      abs_seqno += WRAP_LENGTH;
  return abs_seqno;
}
