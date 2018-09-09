#include <mruby.hpp>

int main()
{
	mruby::VM vm;
    try
    {
	    auto cls = vm.get_class("SomeClass");
    }
    catch (mruby::Exception e)
    {
        return 0;
    }
	return 1;
}
