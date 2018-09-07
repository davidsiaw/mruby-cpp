#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("module SomeModule; end;");
	auto module = vm.get_module("SomeModule");
	return module.get() ? 0 : 1;
}
