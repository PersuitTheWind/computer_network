#include "byte_stream.hh"
using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  return writer_close;
}

void Writer::push( string data )
{
  uint64_t s = available_capacity();
  if ( is_closed() || s <= 0 ) return;
  uint64_t x = min( s, data.size() );
  buffer_.append( data.substr( 0, x ) );
  push_num += x;
  // Your code here.
  //(void)data;
  return;
}

void Writer::close()
{
  writer_close = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return push_num;
}

bool Reader::is_finished() const
{
  return writer_close && buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
  return pop_num;
}

string_view Reader::peek() const
{
  return buffer_;
}

void Reader::pop( uint64_t len )
{
  uint64_t x = min( len, buffer_.size() );
  buffer_.erase( 0, x );
  pop_num += x;
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size();
}
