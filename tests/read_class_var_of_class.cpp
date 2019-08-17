#include <mruby.hpp>

class SomeObject {};

int main()
{
	mruby::VM vm;
	auto cls = vm.create_class<SomeObject>("SomeObject");
	vm.run("class SomeObject; @@a = 5; end;");
	return cls->get_class_variable<int>("@@a") - 5;
}
