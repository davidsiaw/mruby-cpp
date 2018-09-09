#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	auto module = vm.create_module("Animals");
    module->create_module("Felines");
	vm.run("$b = 1; $b = 0 if Animals::Felines.inspect == 'Animals::Felines'");
	return vm.get_global_variable<int>("$b");
}
