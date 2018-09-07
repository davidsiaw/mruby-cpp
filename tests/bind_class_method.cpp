#include <mruby.hpp>

class Cat
{
public:
	static int meow()
	{
		return 500;
	}
};

int main()
{
	mruby::VM vm;
	//auto cls = vm.create_class<Cat>("Cat");
	//cls->bind_method("meow", &Cat::meow);
	//vm.run("$a = Cat.meow");
	return vm.get_global_variable<int>("$a") - 500;
}
