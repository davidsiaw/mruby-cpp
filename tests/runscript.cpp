#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("p 'hello'");
	return EXIT_SUCCESS;
}
