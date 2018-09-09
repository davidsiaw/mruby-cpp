#include <mruby.hpp>

class Cat
{
public:
	Cat(int a)
	{}
};

int main()
{
	mruby::VM vm;
	auto module = vm.create_module("Animals");
	module->create_class<Cat, int>("Cat");
	vm.run("$b = 1; $b = 0 if Animals::Cat.inspect == 'Animals::Cat'");
	return vm.get_global_variable<int>("$b");
}
