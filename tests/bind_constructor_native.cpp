#include <mruby.hpp>

class Meow
{
public:
	int value() const
	{
		return 15;
	}
};

class Cat
{
	mruby::NativeObject<Meow> meow;
public:
	Cat(mruby::NativeObject<Meow> meow) : meow(meow)
	{

	}

	int sound()
	{
		return meow->value();
	}
};

int main()
{
	mruby::VM vm;
	auto meow = vm.create_class<Meow>("Meow");

	auto cls = vm.create_class<Cat, mruby::NativeObject<Meow> >("Cat");
	cls->bind_instance_method("sound", &Cat::sound);

	vm.run("meow = Meow.new; $a = Cat.new(meow).sound");

	return vm.get_global_variable<int>("$a") - 15;
}
