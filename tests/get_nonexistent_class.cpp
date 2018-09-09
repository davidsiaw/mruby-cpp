#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	try
	{
		auto cls = vm.get_class("SomeClass");
	}
	catch (const mruby::NameError &e)
	{
		std::string error(e.what());
		if (error == "NameError: Class does not exist: SomeClass")
			return 0;
	}
	return 1;
}
