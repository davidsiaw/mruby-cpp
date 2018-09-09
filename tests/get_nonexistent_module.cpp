#include <mruby.hpp>

int main()
{
	mruby::VM vm;
    try
    {
	    auto module = vm.get_module("SomeModule");
    }
    catch (mruby::Exception e)
    {
        return 0;
    }
	return 1;
}
