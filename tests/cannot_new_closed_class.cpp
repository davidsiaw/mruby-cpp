#include <iostream>
#include <mruby.hpp>

class Cat
{

};

int main()
{
	mruby::VM vm;
	auto module = vm.create_closed_class<Cat>("Cat");
	vm.run("begin; Cat.new; rescue ArgumentError => e; $err = e.message; end");

	auto msg = vm.get_global_variable<std::string>("$err");
	
	if (msg.find("cannot be created with new", 0) == std::string::npos)
	{
		return 1;
	}
	return 0;
}
