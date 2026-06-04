#pragma once
#include <stdexcept>
#include <string>
using std::string;

class CheckerError : public std::runtime_error {
public:
	explicit CheckerError(const string& error_msg) :
		runtime_error(error_msg) {
	}
};