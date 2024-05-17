#include <mruby.hpp>

#include <string>

int countchars(std::string str)
{
	printf("%s", str.c_str());
	return str.length();
}

int main()
{
	mruby::VM vm;
	vm.bind_method("countchars", &countchars);
	vm.run("$a = countchars 'HELLO!'");
	return vm.get_global_variable<int>("$a") - 6;
}
