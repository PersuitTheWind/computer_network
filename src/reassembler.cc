#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  uint64_t last_index = first_index + data.length() ;
  uint64_t st = first_index , ed = last_index ;
  std::string data1 = data;
  if (last_index <= first_unassembled_index() || first_index >= first_unacceptable_index() || data.empty() ) {
     end_of_a_bytestream ( is_last_substring );
     return;
  }
  if ( st <= first_unassembled_index()){
     data1 = data1.substr(first_unassembled_index() - first_index);
     st = first_unassembled_index();
  }
  if ( ed >= first_unacceptable_index()){
     data1 = data1.substr( 0 ,data1.length() - ( last_index - first_unacceptable_index() ) );
     ed = first_unacceptable_index()；
  }
  
  if (st > first_unassembled_index()){
     
  
     end_of_a_bytestream ( is_last_substring );
  }
  else output_.writer().push( data1 );
  check_buffer();
  end_of_a_bytestream ( is_last_substring );
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return ;
}
uint64_t first_unassembled_index(){
  return output_.writer().bytes_pushed();
}
uint64_t first_unacceptable_index(){
  return output_.writer().bytes_pushed() + output_.writer().available_capacity();
}
void end_of_a_bytestream(bool is_last_substring){
  if (is_last_substring) output_.writer().close;
}
