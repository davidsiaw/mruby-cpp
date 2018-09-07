#include <mruby.hpp>

class Cat
{
	int value;
public:
	Cat(int value) : value(value)
	{

	}

	int meow()
	{
		return value;
	}
};

int main()
{
	mruby::VM vm;
	auto cls = vm.create_class<Cat, int>("Cat");
	cls->bind_instance_method("meow", &Cat::meow);
	vm.run("$a = Cat.new(10).meow");
	return vm.get_global_variable<int>("$a") - 10;
}
