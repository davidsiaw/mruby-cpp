#include <mruby.hpp>

int x = 1;

class Foo
{
public:
	Foo()
	{

	}

	void m()
	{
		x = 0;
	}
};

int main()
{
	mruby::VM vm;

	auto foo = vm.create_class<Foo>("Foo");
	foo->bind_instance_method("m", &Foo::m);

	if (!foo.get()) { return 1; }

	vm.run("Foo.new.m");
	return x;
}
