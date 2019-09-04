#include <mruby.hpp>

int result = 4;

class Point
{
public:
	Point()
	{ result -= 1; }

	~Point()
	{ result -= 1; }
};

class Circle
{
	std::shared_ptr<Point> p;
public:
	Circle(mruby::NativeObject<Point> point) : p(point.get_shared_instance())
	{ result -= 1; }

	~Circle()
	{ result -= 1; }
};

int main()
{
	{
		mruby::VM vm;
		vm.create_class<Point>("Point");
		auto cls = vm.create_class<Circle, mruby::NativeObject<Point>>("Circle");
		vm.run("Circle.new(Point.new)");
	}
	return result;
}
