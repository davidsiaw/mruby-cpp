#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("$a = [0, 1, 2]");
	auto a = vm.get_global_variable<mruby::NativeArray>("$a");
    a->set<std::string>(2, "2");
	return (a->size() == 3
		&& a->get<int>(0) == 0
		&& a->get<int>(1) == 1
		&& a->get<std::string>(2) == "2") ? 0 : 1;
}
