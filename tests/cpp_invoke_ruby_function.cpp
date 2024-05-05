#include <mruby.hpp>

int main()
{
	mruby::VM vm;
	vm.run("$proc = Proc.new do |x|; 10 + x; end;");
	auto proc = vm.get_global_variable< mruby::Function<int(int)> >("$proc");
	auto result = proc.invoke(5);
	return result - 15;
}
