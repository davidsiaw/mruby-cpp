#include <mruby.hpp>

class Cat
{
	int value;
public:
	Cat(int value) : value(value)
	{
		throw mruby::RubyRuntimeError("in class constructor");
	}

	int meow()
	{
		return value;
	}
};

int main()
{
	mruby::VM vm;
	auto cls = vm.create_class<Cat, int>("Cat");
	cls->bind_instance_method("meow", &Cat::meow);

	std::string script = 	"$b = 0"
						"\n""begin"
						"\n""  $b = Cat.new(10)"
						"\n""rescue RuntimeError => e"
						"\n""  if e.message == 'RuntimeError in C binding: in class constructor'"
						"\n""    $b = 1000"
						"\n""  end"
						"\n""end";
	vm.run(script);
	return vm.get_global_variable<int>("$b") - 1000;
}
