#include <mruby.hpp>

int number()
{
	return 100;
}

int main()
{
	mruby::VM vm;
	auto module = vm.create_module("Animals");
	module->bind_method("number", number);
	vm.run("$a = Animals.number");
	return vm.get_global_variable<int>("$a") - 100;
}
