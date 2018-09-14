#include <mruby.hpp>

class Cat
{
public:
	Cat()
	{
		throw mruby::RubyRuntimeError("in class default constructor");
	}
};

int main()
{
	mruby::VM vm;
	auto cls = vm.create_class<Cat>("Cat");

	std::string script = 	"$b = 0"
							"\nbegin"
							"\n  $b = Cat.new"
							"\nrescue RuntimeError => e"
							"\n  if e.message == 'RuntimeError in C binding: in class default constructor'"
							"\n    $b = 1000"
							"\n  end"
							"\nend";
	vm.run(script);
	return vm.get_global_variable<int>("$b") - 1000;
}
