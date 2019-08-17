#include <mruby.hpp>

class Cat
{
	int value;
public:
	Cat() : value(12)
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
	auto cls = vm.create_class<Cat>("Cat");
	cls->bind_instance_method("meow", &Cat::meow);
	vm.run("$a = Cat.new.meow");
	return vm.get_global_variable<int>("$a") - 12;
}
