#include <mruby.hpp>

class Cat
{
public:
	int meow(int a, int b)
	{
		return 5;
	}
};

int main()
{
	mruby::VM vm;
	auto cls = vm.create_class<Cat>("Cat");
	cls->bind_instance_method("meow", &Cat::meow);

	std::string script =	"$b = 0"
						"\n""begin"
						"\n""  $a = Cat.new"
						"\n""  $b = $a.meow 1, 2, 3"
						"\n""rescue ArgumentError => e"
						"\n""  if e.message.start_with?('in \\'#<Cat:0x') and e.message.end_with?('>\\': meow: wrong number of arguments (3 for 2)')"
						"\n""    $b = 1000"
						"\n""  end"
						"\n""end";
	vm.run(script);
	return vm.get_global_variable<int>("$b") - 1000;
}
