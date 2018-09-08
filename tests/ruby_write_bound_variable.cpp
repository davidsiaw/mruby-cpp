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

	vm.run("$pos.x = 333");
	return pos->x - 333;
}
