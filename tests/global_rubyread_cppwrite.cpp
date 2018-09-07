#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.set_global_variable("$a", 100);
	vm.run("$b = 1; $b = 0 if $a == 100");
	return vm.get_global_variable<int>("$b");
}
