#ifndef __MRUBYEXCEPTION_HPP__
#define __MRUBYEXCEPTION_HPP__

#include <string>
#include <sstream>

class Exception : public std::exception
{
public:
	Exception(const std::string &type, const std::string &msg, const std::string &name)
	{
		std::stringstream s;
		s << type << ": " << msg << ": " << name;
		error = s.str();
	}
	const char *what() const noexcept
	{
		return error.c_str();
	}
protected:
	std::string error;
};

// TODO
// For now, defining these exceptions that occur outside the VM
// Will want to separate the exceptions that occur outside from inside

class NameError : public Exception
{
public:
	NameError(const std::string &msg, const std::string &name)
		: Exception("NameError", msg, name)
	{ }
};

class NotImplementedError : public Exception
{
public:
	NotImplementedError(const std::string &msg, const std::string &name)
		: Exception("NotImplementedError", msg, name)
	{ }
};

class TypeError : public Exception
{
public:
	TypeError(const std::string &msg, const std::string &name)
		: Exception("TypeError", msg, name)
	{ }
};

#endif // __MRUBYEXCEPTION_HPP__
