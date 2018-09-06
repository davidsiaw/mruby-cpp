#include <mruby.hpp>

int main()
{
	MRuby mruby;
	mruby.run("p 'hello'");
	return EXIT_SUCCESS;
}
