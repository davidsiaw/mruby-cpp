#include <mruby.hpp>

int result = 2;

class Cat
{
public:
	Cat() { result -= 1; }
	~Cat() { result -= 1; }
};

class Meowery
{
public:
	mruby::NativeObject<Cat> makecat(int)
	{
		auto ptr = std::make_shared<Cat>();
		return mruby::NativeObject<Cat>("Cat", ptr);
	}	
};

int main()
{
	{
		mruby::VM vm;
		vm.create_class<Cat>("Cat");
		auto cls = vm.create_class<Meowery>("Meowery");
		cls->bind_instance_method("makecat", &Meowery::makecat);
		vm.run("$a = Meowery.new.makecat 5");
	}
	return result;
}
