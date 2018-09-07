#ifndef __MRUBYVM_HPP__
#define __MRUBYVM_HPP__

class VM : public Module
{

public:
	VM() : 
		Module(std::shared_ptr<mrb_state>(mrb_open(), mrb_close))
	{
	}

	~VM()
	{
	}

	void run(std::string code)
	{
		std::stringstream ss;
		ss << "begin;";
		ss << code << ";";
		ss << "rescue => e;";
		ss << "p e;";
		ss << "end;";

		mrb_load_string(mrb.get(), ss.str().c_str());
	}
};

#endif // __MRUBYVM_HPP__
