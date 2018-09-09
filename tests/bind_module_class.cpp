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
	auto module = vm.create_module("Animals");
	module->create_class<Cat, int>("Cat");
    auto cls = module->get_class("Cat");
	return cls.get() ? 0 : 1;
}
