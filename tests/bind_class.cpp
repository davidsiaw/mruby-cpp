#include <mruby.hpp>

class Cat
{

};

int main()
{
	mruby::VM vm;
	//vm.create_class<Cat>("Cat");
	auto cls = vm.get_class("Cat");
	return cls.get() ? 0 : 1;
}
