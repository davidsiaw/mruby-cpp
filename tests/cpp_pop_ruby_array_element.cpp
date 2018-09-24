#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("$a = [0, 1, 2]");
	auto a = vm.get_global_variable<mruby::NativeArray>("$a");
	return (a->pop<int>() == 2
		&& a->size() == 2
		&& a->get<int>(0) == 0
		&& a->get<int>(1) == 1) ? 0 : 1;
}
