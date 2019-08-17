#include <mruby.hpp>

class Foo
{
public:
	Foo()
	{

	}

	int m()
	{
		return 5;
	}
};

int main()
{
	mruby::VM vm;

	auto foo = vm.create_class<Foo>("Foo");
	foo->bind_instance_method("m", &Foo::m);

	if (!foo.get()) { return 1; }

	vm.run("$a = Foo.new.m");
	return vm.get_global_variable<int>("$a") - 5;
}
