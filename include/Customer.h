#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
class customer {
  private:
	std::string name;                 // customer Name
	uint64_t national_id;             // customer national ID
	std::vector<uint64_t> account_id; // in case customer has multiple accounts

  public:
	customer(std::string name, uint64_t national_id);
	bool addAccountID(uint64_t account_id);
	bool removeAccount(uint64_t account_id);
	std::vector<uint64_t> getAccountIDs() const;
	std::string getName() const;
	bool setName(const std::string &newName);
	uint64_t getNationalID() const;
};