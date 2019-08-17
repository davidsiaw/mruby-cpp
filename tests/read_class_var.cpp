#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("@@a = 2000; p @@a");
	return vm.get_class_variable<int>("@@a") - 2000;
}
