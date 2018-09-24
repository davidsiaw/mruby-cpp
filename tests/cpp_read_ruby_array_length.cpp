#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("$a = [0, 1, 2]");
	auto a = vm.get_global_variable<mruby::NativeArray>("$a");
	return a->length() - 3;
}
