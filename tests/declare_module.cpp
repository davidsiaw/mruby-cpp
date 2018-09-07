#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.create_module("Play");
	vm.run("$b = 1; $b = 0 if Play.inspect == 'Play'");
	return vm.get_global_variable<int>("$b");
}
