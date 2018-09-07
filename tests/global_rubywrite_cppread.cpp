#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("$a = 100;");
	return vm.get_global_variable<int>("$a") - 100;
}
