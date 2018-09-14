#include <mruby.hpp>

int number()
{
	throw mruby::RubyRuntimeError("in module method");
	return 100;
}

int main()
{
	mruby::VM vm;
	auto module = vm.create_module("Animals");
	module->bind_method("number", number);

	std::string script =	"$b = 0"
							"\nbegin"
							"\n  $b = Animals.number"
							"\nrescue RuntimeError => e"
							"\n  if e.message == 'RuntimeError in C binding: in module method'"
							"\n    $b = 1000"
							"\n  end"
							"\nend";
	vm.run(script);
	return vm.get_global_variable<int>("$b") - 1000;
}

