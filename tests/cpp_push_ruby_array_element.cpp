#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("$a = [0, 1, 2]");
	auto a = vm.get_global_variable<mruby::NativeArray>("$a");
	a->push<int>(3);
	return (a->size() == 4
		&& a->get<int>(0) == 0
		&& a->get<int>(1) == 1
		&& a->get<int>(2) == 2
		&& a->get<int>(3) == 3) ? 0 : 1;
}
