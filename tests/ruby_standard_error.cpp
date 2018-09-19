#include <mruby.hpp>

int number()
{
	throw mruby::RubyStandardError();
	return 100;
}

int main()
{
	mruby::VM vm;
	auto module = vm.create_module("Animals");
	module->bind_method("number", number);

	std::string script =	"$b = 0"
						"\n""begin"
						"\n""  $b = Animals.number"
						"\n""rescue RuntimeError => e"
						"\n""  if e.message == 'StandardError in C binding'"
						"\n""    $b = 1000"
						"\n""  end"
						"\n""end";
	vm.run(script);
	return vm.get_global_variable<int>("$b") - 1000;
}

