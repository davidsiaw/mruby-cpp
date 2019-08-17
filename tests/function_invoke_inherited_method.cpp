#include <mruby.hpp>

class Foo
{
public:
	Foo()
	{

	}

	int m(int a)
	{
		return a + 5;
	}
};

class Bar : public Foo
{
};

int main()
{
	mruby::VM vm;

	auto foo = vm.create_class<Bar>("Bar");
	foo->bind_instance_method("m", &Bar::m);

	if (!foo.get()) { return 1; }

	vm.run("$a = Bar.new.m(1)");
	return vm.get_global_variable<int>("$a") - 6;
}
