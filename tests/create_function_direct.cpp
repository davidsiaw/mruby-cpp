#include <mruby.hpp>

int triple(int num)
{
	return num * 3;
}

int main()
{
	mruby::VM vm;
	vm.bind_method("triple", &triple);
	vm.run("$a = triple 2");
	return vm.get_global_variable<int>("$a") - 6;
}
