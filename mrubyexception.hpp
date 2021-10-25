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

	virtual ~Exception()
	{ }

	const char *what() const noexcept
	{
		return error.c_str();
	}
protected:
	std::string error;
};

// Exceptions that occur outside the VM

class NameError : public Exception
{
public:
	NameError(const std::string &msg, const std::string &name)
		: Exception("NameError", msg, name)
	{ }
	
	virtual ~NameError()
	{ }

};

class NotImplementedError : public Exception
{
public:
	NotImplementedError(const std::string &msg, const std::string &name)
		: Exception("NotImplementedError", msg, name)
	{ }
	
	virtual ~NotImplementedError()
	{ }

};

class TypeError : public Exception
{
public:
	TypeError(const std::string &msg, const std::string &name)
		: Exception("TypeError", msg, name)
	{ }

	virtual ~TypeError()
	{ }

};

class ArgumentError : public Exception
{
public:
	ArgumentError(const std::string &msg, const std::string &name)
		: Exception("ArgumentError", msg, name)
	{ }

	virtual ~ArgumentError()
	{ }

};

// Exceptions that occur inside the VM
class RubyException : public std::exception
{
public:
	RubyException()
		: error("Exception in C binding")
	{ }
	
	RubyException(const std::string &type, const std::string &msg)
	{
		std::stringstream s;
		s << type << " in C binding";
		if (msg != "")
		{
			s << ": " << msg;
		}
		error = s.str();
	}

	virtual ~RubyException()
	{ }

	const char *what() const noexcept
	{
		return error.c_str();
	}
protected:
	std::string error;
};

class RubyStandardError : public RubyException
{
public:
	RubyStandardError(const std::string &msg="")
		: RubyException("StandardError", msg)
	{ }

	virtual ~RubyStandardError()
	{ }

protected:
	RubyStandardError(const std::string &type, const std::string &msg)
		: RubyException(type, msg)
	{ }
};

class RubyRuntimeError : public RubyStandardError
{
public:
	RubyRuntimeError(const std::string &msg="")
		: RubyStandardError("RuntimeError", msg)
	{ }

	virtual ~RubyRuntimeError()
	{ }

};
#endif // __MRUBYEXCEPTION_HPP__
