#include <mruby.hpp>

class Foo
{
public:
	int m;
	Foo() : m(5)
	{

	}
};

class Bar : public Foo
{
public:
	Bar() : Foo()
	{

	}
};

int main()
{
	mruby::VM vm;

	auto bar = vm.create_class<Bar>("Bar");
	bar->bind_instance_variable("m", &Bar::m);

	return 0;
}
