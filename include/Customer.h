#pragma once
#include<iostream>
#include<string>
#include  <cstdint>
#include<vector>
class customer {
    private:
  std:: string name;  //customer Name
  uint64_t national_id; //customer national ID
 std::vector<uint64_t>account_id; //in case customer has multiple accounts

 public:
 customer(std::string name,uint64_t national_id);
 void addAccountID(uint64_t account_id);
 void removeAccount(uint64_t account_id);
 std::vector<uint64_t>getAccountIDs();
 std::string getName();
 uint64_t getNationalID();
    
};