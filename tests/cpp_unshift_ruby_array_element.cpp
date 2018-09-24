#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("$a = [0, 1, 2]");
	auto a = vm.get_global_variable<mruby::NativeArray>("$a");
	a->unshift<int>(5);
	return (a->size() == 4
		&& a->get<int>(0) == 5
		&& a->get<int>(1) == 0
		&& a->get<int>(2) == 1
		&& a->get<int>(3) == 2) ? 0 : 1;
}
