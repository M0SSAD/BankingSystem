#include "Customer.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

customer::customer(std::string Name, uint64_t id)
    : name(Name), national_id(id) {}

// Modification functions (Setters)
bool customer::setName(const std::string &newName) {
	if (!newName.empty()) {
		name = newName;
		return true;
	} else {
		return false;
	}
}

bool customer::addAccountID(uint64_t id) {
	// check if that id already exist
	for (const auto &existing_id : account_id) {
		if (existing_id == id) {
			return false;
		}
	}
	account_id.push_back(id);
	return true;
}
bool customer::removeAccount(uint64_t id) {
	for (auto it = account_id.begin(); it != account_id.end(); ++it) {
		if (*it == id) {
			account_id.erase(it);
			return true;
		}
	}
	return false; // Not found
}
std::vector<uint64_t> customer::getAccountIDs() const { return account_id; }
std::string customer::getName() const { return name; }
uint64_t customer::getNationalID() const { return national_id; }
