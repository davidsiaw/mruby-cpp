#include <mruby.hpp>

class InternalObj
{
public:
	int a;
	InternalObj(int a): a(a)
	{ }
};

class TestClass
{
public:
	mruby::NativeObject<InternalObj> obj;
	TestClass() : obj("InternalObj", std::make_shared<InternalObj>(0))
	{ }

	int num()
	{
		obj->a;
	}
};

int main()
{
	mruby::VM vm;

	auto intern = vm.create_class<InternalObj, int>("InternalObj");
	intern->bind_instance_variable("a", &InternalObj::a);

	auto cls = vm.create_class<TestClass>("TestClass");
	cls->bind_instance_variable("obj", &TestClass::obj);
	cls->bind_instance_method("num", &TestClass::num);

	vm.run(
		"io = InternalObj.new(5);"
		"tc = TestClass.new;"
		"tc.obj = io;"
		"$a = tc.num;"
		"p $a"
		);
	return vm.get_global_variable<int>("$a") - 5;
}
