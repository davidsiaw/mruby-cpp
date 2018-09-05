#pragma once

#ifdef _WIN32
#include <stdafx.h>
#endif

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <mruby/variable.h>
#include <mruby/string.h>
#include <memory>
#include "initexception.hpp"

class MRubyClass;

template<typename T>
class MRubyTypeBinder {
public:
};

template<>
class MRubyTypeBinder<int> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, int i) { return mrb_fixnum_value(i); }
	static int from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_fixnum(val); }
};

template<>
class MRubyTypeBinder<const char*> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, std::string str) { return mrb_str_new(mrb, str.c_str(), str.size()); }
	static std::string from_mrb_value(mrb_state* mrb, mrb_value val) { return std::string(RSTRING_PTR(val), RSTRING_LEN(val)); }
};

template<>
class MRubyTypeBinder<std::string> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, std::string str) { return mrb_str_new(mrb, str.c_str(), str.size()); }
	static std::string from_mrb_value(mrb_state* mrb, mrb_value val) { return std::string(RSTRING_PTR(val), RSTRING_LEN(val)); }
};

template<>
class MRubyTypeBinder<RClass*> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, RClass* cls) { return mrb_class_find_path(mrb, cls); }
	static RClass* from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_class(mrb, val); }
};

template<>
class MRubyTypeBinder<RData*> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, RData* data) { 
		mrb_value val = { 0 };
		val.tt = data->tt;
		val.value.p = data;
		return val;
	}
	static RData* from_mrb_value(mrb_state* mrb, mrb_value val) { return RDATA(val); }
};


template<typename T>
mrb_value get_value_from(mrb_state* mrb, T val)
{
	return MRubyTypeBinder<T>::to_mrb_value(mrb, val);
}

template<typename T>
T get_object_from(mrb_state* mrb, mrb_value val)
{
	return MRubyTypeBinder<T>::from_mrb_value(mrb, val);
}


class MRubyModule
{
protected:
	std::shared_ptr<mrb_state> mrb;

	MRubyModule(std::shared_ptr<mrb_state> mrb) :
		MRubyModule(mrb, "", nullptr)
	{

	}

private:
	RClass* cls;
	std::string name;

public:
	MRubyModule(std::shared_ptr<mrb_state> mrb, std::string name, RClass* cls) :
		mrb(mrb),
		cls(cls), 
		name(name)
	{

	}

	~MRubyModule()
	{

	}

	bool thing_is_defined(std::string name, mrb_vtype type)
	{
		mrb_sym name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			mrb_value val = mrb_vm_const_get(mrb.get(), name_sym);
			return val.tt == type;
		}
		bool is_defined = mrb_const_defined(mrb.get(), mrb_obj_value(cls), name_sym) == 1;
		if (is_defined)
		{
			mrb_value val = mrb_const_get(mrb.get(), mrb_obj_value(cls), name_sym);
			return val.tt == type;
		}
		return false;
	}

	std::shared_ptr<MRubyModule> get_class(std::string name)
	{
		if (!thing_is_defined(name, MRB_TT_CLASS))
		{
			throw InitException("Module does not exist", name);
		}
		RClass *theclass = NULL;
		mrb_sym name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		mrb_value m = mrb_vm_const_get(mrb.get(), name_sym);
		return std::make_shared<MRubyModule>(mrb, name, theclass);
	}

	std::shared_ptr<MRubyModule> get_module(std::string name)
	{
		if (!thing_is_defined(name, MRB_TT_MODULE))
		{
			throw InitException("Module does not exist", name);
		}
		RClass *theclass = NULL;
		mrb_sym name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		mrb_value m = mrb_vm_const_get(mrb.get(), name_sym);
		return std::make_shared<MRubyModule>(mrb, name, theclass);
	}

	std::shared_ptr<MRubyModule> create_module(std::string name)
	{
		if (cls == nullptr)
		{
			RClass* topmodule = mrb_define_module(mrb.get(), name.c_str());
			return std::make_shared<MRubyModule>(mrb, name, topmodule);
		}

		RClass* submodule = mrb_define_module_under(mrb.get(), cls, name.c_str());
		return std::make_shared<MRubyModule>(mrb, name, submodule);

		//mrb_data_type type;
		//type.dfree = MRubyObject<T>::free_object;
		//type.struct_name = "";
		//Data_Make_Struct(mrb.get(), )
	}

	std::shared_ptr<MRubyClass> create_class(std::string name)
	{
		if (cls == nullptr)
		{
			RClass* topclass = mrb_define_class(mrb.get(), name.c_str(), mrb->object_class);
			return std::make_shared<MRubyClass>(mrb, name, topclass);
		}

		RClass* innerclass = mrb_define_class_under(mrb.get(), cls, name.c_str(), mrb->object_class);
		return std::make_shared<MRubyClass>(mrb, name, innerclass);
	}

	template<typename T>
	void set_class_variable(std::string name, T value)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			mrb_vm_iv_set(mrb.get(), var_name_sym, MRubyTypeBinder<T>::to_mrb_value(mrb.get(), value));
		}
		else
		{
			mrb_iv_set(mrb.get(), mrb_obj_value(cls), var_name_sym, MRubyTypeBinder<T>::to_mrb_value(mrb.get(), value));
		}
	}

	template<typename T>
	T get_class_variable(std::string name)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			return MRubyTypeBinder<T>::from_mrb_value(mrb.get(), mrb_vm_iv_get(mrb.get(), var_name_sym));
		}
		else
		{
			return MRubyTypeBinder<T>::from_mrb_value(mrb.get(), mrb_iv_get(mrb.get(), mrb_obj_value(cls), var_name_sym));
		}
	}

	template<typename T>
	void set_global_variable(std::string name, T value)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		mrb_gv_set(mrb.get(), var_name_sym, MRubyTypeBinder<T>::to_mrb_value(mrb.get(), value));
	}

	template<typename T>
	T get_global_variable(std::string name)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		return MRubyTypeBinder<T>::from_mrb_value(mrb.get(), mrb_gv_set(mrb.get(), var_name_sym));
	}

	RProc* create_function(mrb_func_t func)
	{
		RProc* proc = mrb_proc_new_cfunc(mrb.get(), func);
		return proc;
	}



	template< class Func >
	void set_function(const char* name, Func func)
	{
		mrb_sym func_s = mrb_intern_cstr(mrb, name);
		mrb_value env[] = {
			mrb_cptr_value(mrb, (void*)func),  // 0: c function pointer
			mrb_symbol_value(func_s),          // 1: function name
		};
		RProc* func_proc = mrb_proc_new_cfunc_with_env(mrb, sfunc, 2, env);
		auto kernelmod = mrb->kernel_module;
		mrb_method_t method;
		MRB_METHOD_FROM_PROC(method, func_proc);
		mrb_define_method_raw(mrb, kernelmod, func_s, method);
	}

};

class MRubyClass : public MRubyModule
{
	static void destructor(mrb_state* mrb, void* ptr)
	{
		printf("destructor called %x\n", ptr);
	}

	static mrb_value constructor(mrb_state* mrb, mrb_value self)
	{
		mrb_data_type type;
		type.dfree = destructor;
		type.struct_name = "";
		int* ptr = new int;
		printf("new called %x\n", ptr);
		auto obj = mrb_data_object_alloc(mrb, get_object_from<RClass*>(mrb, self), ptr, &type);
		return get_value_from(mrb, obj);
	}

public:

	MRubyClass(std::shared_ptr<mrb_state> mrb, std::string name, RClass* cls) :
		MRubyModule(mrb, name, cls)
	{
		mrb_define_class_method(mrb.get(), cls, "new", constructor, MRB_ARGS_ARG(0,0));
	}

	~MRubyClass()
	{

	}

};


class MRuby : public MRubyModule
{

public:
	MRuby() : 
		MRubyModule(std::shared_ptr<mrb_state>(mrb_open(), mrb_close))
	{
	}

	~MRuby()
	{
	}

	void run(std::string code)
	{
		mrb_load_string(mrb.get(), code.c_str());
	}
};
