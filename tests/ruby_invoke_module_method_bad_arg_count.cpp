#include <mruby.hpp>

int number(int a, int b, int c)
{
	return 100;
}

int main()
{
	mruby::VM vm;
	auto module = vm.create_module("Animals");
	module->bind_method("number", number);

	std::string script =    "$b = 0"
						"\n""begin"
						"\n""  $b = Animals.number 1"
						"\n""rescue ArgumentError => e"
						"\n""  if e.message == 'in \\'Animals\\': number: wrong number of arguments (1 for 3)'"
						"\n""    $b = 1000"
						"\n""  end"
						"\n""end";
	vm.run(script);
	return vm.get_global_variable<int>("$b") - 1000;
}
