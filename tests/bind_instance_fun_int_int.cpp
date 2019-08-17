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

int main()
{
	mruby::VM vm;

	auto foo = vm.create_class<Foo>("Foo");
	foo->bind_instance_method("m", &Foo::m);

	return 0;
}
