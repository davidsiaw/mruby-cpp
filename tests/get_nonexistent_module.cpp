#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	try
	{
		auto module = vm.get_module("SomeModule");
	}
	catch (const mruby::NameError &e)
	{
		std::string error(e.what());
		if (error == "NameError: Module does not exist: SomeModule")
			return 0;
	}
	return 1;
}
