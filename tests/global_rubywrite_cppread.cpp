#include <mruby.hpp>

int main()
{
	MRuby mruby;
	mruby.run("$a = 100;");
	return mruby.get_global_variable<int>("$a") - 100;
}
