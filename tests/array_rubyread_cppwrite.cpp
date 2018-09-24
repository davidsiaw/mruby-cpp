#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	mruby::NativeArray a;
	vm.set_global_variable("$a", a);
	a->push(100);
	vm.run("$b = 1; $b = 0 if $a.first == 100");
	return vm.get_global_variable<int>("$b");
}
