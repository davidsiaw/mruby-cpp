#include <mruby.hpp>

class Foo
{
public:
	static void m()
	{
	}
};

int main()
{
	mruby::VM vm;

	auto foo = vm.create_class<Foo>("Foo");
	foo->bind_method("m", &Foo::m);

	return 0;
}
