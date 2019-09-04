#include <mruby.hpp>

int result = 2;

class Meow
{
public:
	Meow() { result -= 1; }
	~Meow() { result -= 1; }
};

class Scratch
{
	std::shared_ptr<Meow> meow;
public:
	Scratch(std::shared_ptr<Meow> meow) : meow(meow) { }
	~Scratch() { }

};

class Cat
{
	std::shared_ptr<Meow> meow;
public:
	Cat(std::shared_ptr<Meow> meow) : meow(meow) { }
	~Cat() { }

	mruby::NativeObject<Scratch> scratch()
	{
		auto ptr = std::make_shared<Scratch>(meow);
		return mruby::NativeObject<Scratch>("Scratch", ptr);
	}
};

class Meowery
{
	std::shared_ptr<Meow> meow;
public:
	Meowery() : meow(std::make_shared<Meow>())
	{ }

	mruby::NativeObject<Cat> makecat(int)
	{
		auto ptr = std::make_shared<Cat>(meow);
		return mruby::NativeObject<Cat>("Cat", ptr);
	}
};

int main()
{
	{
		mruby::VM vm;
		vm.create_closed_class<Scratch>("Scratch");

		auto cat = vm.create_closed_class<Cat>("Cat");
		cat->bind_instance_method("scratch", &Cat::scratch);

		auto cls = vm.create_class<Meowery>("Meowery");
		cls->bind_instance_method("makecat", &Meowery::makecat);
		vm.run("m = Meowery.new; cat = m.makecat 5; v = cat.scratch");
	}
	return result;
}
