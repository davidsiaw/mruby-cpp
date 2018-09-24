#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("$a = [5, 1, 2]");
	auto a = vm.get_global_variable<mruby::NativeArray>("$a");
	return (a->shift<int>() == 5
		&& a->size() == 2
		&& a->get<int>(0) == 1
		&& a->get<int>(1) == 2) ? 0 : 1;
}
