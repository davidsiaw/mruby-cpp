#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("@a = 2000;");
	return vm.get_instance_variable<int>("@a") - 2000;
}
