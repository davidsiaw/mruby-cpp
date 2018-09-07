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

	void run(const std::string& code)
	{
		std::stringstream ss;
		ss << "begin;";
		ss << code << ";";
		ss << "rescue => e;";
		ss << "p e;";
		ss << "end;";

		mrb_load_string(mrb.get(), ss.str().c_str());
	}

	void run_file(const std::string& file)
	{
		std::shared_ptr<mrbc_context> cxt(mrbc_context_new(mrb.get()), [=](mrbc_context* p) {mrbc_context_free(mrb.get(), p);});
		mrbc_filename(mrb.get(), cxt.get(), file.c_str());

		std::ifstream t(file);
		std::string source((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		mrb_load_nstring_cxt(mrb.get(), source.c_str(), source.length(), cxt.get());
	}
};

#endif // __MRUBYVM_HPP__
