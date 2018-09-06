#include <mruby.hpp>

int main()
{
	MRuby mruby;
	mruby.set_global_variable("$a", 100);
	mruby.run("$b = 1; $b = 0 if $a == 100");
	return mruby.get_global_variable<int>("$b");
}
