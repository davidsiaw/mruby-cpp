#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("class SomeClass; end;");
	auto cls = vm.get_class("SomeClass");
	return cls.get() ? 0 : 1;
}
