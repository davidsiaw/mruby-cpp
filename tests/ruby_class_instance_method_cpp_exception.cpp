#include <mruby.hpp>

class Cat
{
	int value;
public:
	Cat(int value) : value(value)
	{
	}

	int meow()
	{
		throw mruby::RubyRuntimeError("in class instance method");
		return value;
	}
};

int main()
{
	mruby::VM vm;
	auto cls = vm.create_class<Cat, int>("Cat");
	cls->bind_instance_method("meow", &Cat::meow);

	std::string script = 	"$b = 0"
							"\nbegin"
							"\n  $a = Cat.new(10)"
							"\n  $b = $a.meow"
							"\nrescue RuntimeError => e"
							"\n  if e.message == 'RuntimeError in C binding: in class instance method'"
							"\n    $b = 1000"
							"\n  end"
							"\nend";
	vm.run(script);
	return vm.get_global_variable<int>("$b") - 1000;
}
