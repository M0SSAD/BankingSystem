#include "Customer.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

Customer::Customer(std::string newName, uint64_t id)
    : name(std::move(newName)), national_id(id) {}

bool Customer::setName(const std::string &newName) {
	if (!newName.empty()) {
		name = newName;
		return true;
	} else {
		return false;
	}
}

bool Customer::addAccountID(uint64_t id) {
	account_id.push_back(id);
	return true;
}
bool Customer::removeAccount(uint64_t id) {
	auto it = std::find(account_id.begin(), account_id.end(), id);
	if (it == account_id.end()) {
		return false; // Not found
	}

	account_id.erase(it);
	return true;
}
std::vector<uint64_t> Customer::getAccountIDs() const { return account_id; }
std::string Customer::getName() const { return name; }
uint64_t Customer::getNationalID() const { return national_id; }
