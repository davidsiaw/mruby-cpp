#include <mruby.hpp>

int x = 0;

class Foo
{
public:
	static void m(int a)
	{
		x = 5;
	}
};

int main()
{
	mruby::VM vm;

	auto foo = vm.create_class<Foo>("Foo");
	foo->bind_method("m", &Foo::m);

	return 0;
}
