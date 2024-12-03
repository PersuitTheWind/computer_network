#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  uint64_t last_index = first_index + data.length() ;
  uint64_t st = first_index , ed = last_index ;
  if (is_last_substring) ed_index = ed;
  if (first_unassembled_index() == ed_index ) { output_.writer().close(); return; }
  std::string data1 = data;
  if (last_index < first_unassembled_index() || first_index >= first_unacceptable_index() || data.empty() ) {
    // end_of_a_bytestream ();
     return;
  }
  if ( st <= first_unassembled_index()){
     data1 = data1.substr(first_unassembled_index() - first_index);
     st = first_unassembled_index();
  }
  if ( ed > first_unacceptable_index()){
     data1 = data1.substr( 0 ,first_unacceptable_index() - st );
     ed = st + data1.length();
  }
  
  if (st > first_unassembled_index()){
     if (! buffer_.empty()){
        auto it = buffer_.upper_bound (st);
        if (it != buffer_.begin()) it--;
        uint64_t st0 = it -> first;
        uint64_t ed0 = st0 + it -> second.length();
        if(st0 <= st && ed0 >= st){
           if(ed0 < ed) data1 = data1.substr(ed0 - st);
           else {
              end_of_a_bytestream ( );
              return;
           }
           data1 = it -> second + data1;
           st = it -> first;
           it = buffer_ . erase(it);
        }
        it = buffer_.upper_bound (st);
        while(it != buffer_.end()){
            if ( ed < it -> first ) break;
            if ( ed <= it -> first + it -> second.length()){
                data1 += it->second.substr(ed - it -> first);
                ed = st + data1.length(); 
            }
            it = buffer_ . erase(it);
        }
     }
     buffer_.insert({ st , data1});    
     //end_of_a_bytestream ();
     return;
  }
  else output_.writer().push( data1 );
  check_buffer();
  end_of_a_bytestream ();
}

uint64_t Reassembler::bytes_pending() const
{
  uint64_t ans = 0;
  for ( auto &it : buffer_ )
     ans += it.second.length();
  return ans;
}
uint64_t Reassembler::first_unassembled_index(){
  return output_.writer().bytes_pushed();
}
uint64_t Reassembler::first_unacceptable_index(){
  return output_.writer().bytes_pushed() + output_.writer().available_capacity();
}
void Reassembler::check_buffer(){
    auto it  = buffer_.begin();
    while( it != buffer_.end() && it -> first <= first_unassembled_index()){
          if ( it -> first + it -> second.length() >= first_unassembled_index()){
               std::string ss = it -> second ;
               ss = ss.substr(first_unassembled_index() - it -> first);
               output_.writer().push(ss);
          }
      
          it = buffer_.erase(it);
    }       
    it = buffer_.begin();
    if (!buffer_.empty() && it -> first <= first_unassembled_index() ){
       std::string ss = it -> second ;
       ss = ss.substr(first_unassembled_index() - it -> first);
       output_.writer().push(ss);
       buffer_.erase ( buffer_.begin());
    }
}
void Reassembler::end_of_a_bytestream(){
  if (buffer_.empty() && first_unassembled_index() == ed_index ) output_.writer().close();
}
