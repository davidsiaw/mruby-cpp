#include <mruby.hpp>

class Cat
{
public:
	Cat(int a)
	{}
};

int main()
{
	mruby::VM vm;
	vm.create_class<Cat,int>("Cat");
	auto cls = vm.get_class("Cat");
	return cls.get() ? 0 : 1;
}
