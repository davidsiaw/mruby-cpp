#include <mruby.hpp>

class Position
{
public:
	int x,y;
};

class Pospos : public Position
{
};

int main()
{
	mruby::VM vm;
	auto cls = vm.create_class<Pospos>("Pospos");
	cls->bind_instance_variable("x", &Pospos::x);
	cls->bind_instance_variable("y", &Pospos::y);

	auto pos = std::make_shared<Pospos>();
	vm.set_global_variable("$pos", mruby::NativeObject<Pospos>("Pospos", pos));

	vm.run("$pos.x = 333");
	return pos->x - 333;
}
