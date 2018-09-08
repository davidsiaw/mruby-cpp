#include <mruby.hpp>

class Position
{
public:
	int x,y;
};

int main()
{
	mruby::VM vm;
	auto cls = vm.create_class<Position>("Position");
	cls->bind_instance_variable("x", &Position::x);
	cls->bind_instance_variable("y", &Position::y);

	auto pos = std::make_shared<Position>();
	vm.set_global_variable("$pos", mruby::NativeObject<Position>("Position", pos));
	pos->x = 222;

	vm.run("$result = $pos.x");
	return vm.get_global_variable<int>("$result") - 222;
}
