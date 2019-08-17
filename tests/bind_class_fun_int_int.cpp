#include <mruby.hpp>

class Foo
{
public:
	static int m(int a)
	{
		return a;
	}
};

int main()
{
	mruby::VM vm;

	auto foo = vm.create_class<Foo>("Foo");
	foo->bind_method("m", &Foo::m);

	return 0;
}
