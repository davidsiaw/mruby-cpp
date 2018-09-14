#include <mruby.hpp>

class Cat
{
public:
	static int meow()
	{
		throw mruby::RubyRuntimeError("in class method");
		return 500;
	}
};

int main()
{
	mruby::VM vm;
	auto cls = vm.create_class<Cat>("Cat");
	cls->bind_method("meow", &Cat::meow);

	std::string script = 	"$b = 0"
						"\n""begin"
						"\n""  $b = Cat.meow"
						"\n""rescue RuntimeError => e"
						"\n""  if e.message == 'RuntimeError in C binding: in class method'"
						"\n""    $b = 1000"
						"\n""  end"
						"\n""end";
	vm.run(script);
	return vm.get_global_variable<int>("$b") - 1000;
}
