#include <mruby.hpp>

int x = 100;

class Foo
{
public:
	Foo()
	{

	}

	void m(int a)
	{
		x = a;
	}
};

int main()
{
	mruby::VM vm;

	auto foo = vm.create_class<Foo>("Foo");
	foo->bind_instance_method("m", &Foo::m);

	return 0;
}
