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
     if (!buffer_.empty()){
        auto it = buffer_.upper_bound (st);
        it--;
        st0 = it -> first;
        ed0 = st0 + it -> second.length();
        if(ed0 >= st + 1){
           if(ed0 < ed) data1 = data1.substr(ed0 - st + 1);
           else {
              end_of_a_bytestream ( is_last_substring );
              return;
           }
           data1 = it -> second + data1;
           st = it -> first;
           it = buffer_ . erase(it);
        }
        else it++;
        while(it != buffer_.end()){
            if ( ed < it -> first ) break;
            if ( ed < it -> first + it -> second.length()){
                data1 += it->second.substr(ed - it -> first);
                ed = it -> first + it -> second.length(); 
            }
            it = buffer_ . erase(it);
        }
     }
     buffer_.insert({ st , data1});    
     end_of_a_bytestream ( is_last_substring );
  }
  else output_.writer().push( data1 );
  check_buffer();
  end_of_a_bytestream ( is_last_substring );
}

uint64_t Reassembler::bytes_pending() const
{
  uint64_t ans = 0;
  for ( auto it : buffer_ ) 
     ans += it -> second.length();
  return ans;
}
uint64_t first_unassembled_index(){
  return output_.writer().bytes_pushed();
}
uint64_t first_unacceptable_index(){
  return output_.writer().bytes_pushed() + output_.writer().available_capacity();
}
void check_buffer(){
    auto it  = buffer_.begin();
    while( it != buffer_.end() && it -> first <= first_unassembled_index()){
          if ( it -> first + it -> second.length() > first_unassembled_index()){
               std::string ss = it -> second ;
               ss = ss.substr(first_unassembled_index() - it -> first);
               output_.writer().push(ss);
          }
          it = buffer_.erase(it);
    }         
}
void end_of_a_bytestream(bool is_last_substring){
  if (is_last_substring) output_.writer().close;
}
