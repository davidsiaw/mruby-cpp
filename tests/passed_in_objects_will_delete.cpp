#include <mruby.hpp>

int result = 2;

class Cat
{
public:
	Cat() { result -= 1; }
	~Cat() { result -= 1; }
};

mruby::NativeObject<Cat> testmakeobj()
{
	auto ptr = std::make_shared<Cat>();
	return mruby::NativeObject<Cat>("Cat", ptr);
}

int main()
{
	{
		mruby::VM vm;
		vm.create_class<Cat>("Cat");
		vm.bind_method("testmakeobj", testmakeobj);
		vm.run("$a = testmakeobj");
	}
	return result;
}
