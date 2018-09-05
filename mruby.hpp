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

template<class TClass>
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
class MRubyTypeBinder<size_t> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, size_t i) { return mrb_fixnum_value(i); }
	static size_t from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_fixnum(val); }
};

template<>
class MRubyTypeBinder<std::string> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, std::string str) { return mrb_str_new(mrb, str.c_str(), str.size()); }
	static std::string from_mrb_value(mrb_state* mrb, mrb_value val) 
	{ 
		if (val.tt == MRB_TT_SYMBOL)
		{
			val = mrb_sym2str(mrb, val.value.i);
		}
		return std::string(RSTRING_PTR(val), RSTRING_LEN(val)); 
	}
};

template<>
class MRubyTypeBinder<const char*> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, std::string str) { return MRubyTypeBinder<std::string>::to_mrb_value(mrb, str); }
	static std::string from_mrb_value(mrb_state* mrb, mrb_value val) { return MRubyTypeBinder<std::string>::from_mrb_value(mrb, val); }
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

	}

	template<typename TClass>
	std::shared_ptr< MRubyClass<TClass> > create_class(std::string name)
	{
		if (cls == nullptr)
		{
			RClass* topclass = mrb_define_class(mrb.get(), name.c_str(), mrb->object_class);
			return std::make_shared<MRubyClass<TClass>>(mrb, name, topclass);
		}

		RClass* innerclass = mrb_define_class_under(mrb.get(), cls, name.c_str(), mrb->object_class);
		return std::make_shared<MRubyClass<TClass>>(mrb, name, innerclass);
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


	template<typename TFunc> 
	struct currier;

	template<typename TRet, typename TArg> 
	struct currier< std::function<TRet(TArg)> >
	{
		using type = std::function<TRet(TArg)>;
		const type result;

		currier(type fun) : result(fun) {}
	};

	template<typename TRet, typename TArgHead, typename ...TArgs> 
	struct currier< std::function<TRet(TArgHead, TArgs...)> >
	{
		using remaining_type = typename currier< std::function<TRet(TArgs...)> >::type;
		using type = std::function<remaining_type(TArgHead)>;

		const type result;

		currier(std::function<TRet(TArgHead, TArgs...)> fun) : result(
			[=](const TArgHead& t)
			{
				return currier< std::function<TRet(TArgs...)> >(
					[=](const TArgs&... ts)
					{
					return fun(t, ts...);
					}
				).result;
			}
		) {}
	};

	template <typename TRet, typename ...TArgs> 
	static auto curry(const std::function<TRet(TArgs...)> fun)
		-> typename currier< std::function< TRet(TArgs...) > >::type
	{
		return currier< std::function< TRet(TArgs...) > >(fun).result;
	}

	template <typename TRet, typename ...TArgs>
	static auto curry(TRet(* const fun)(TArgs...))
		-> typename currier< std::function< TRet(TArgs...) > >::type
	{
		return currier< std::function< TRet(TArgs...) > >(fun).result;
	}

	template <int idx, typename TRet>
	static TRet func_caller(mrb_state* mrb, TRet t, mrb_value* args)
	{
		return t;
	}

	template <int idx, typename TRet, typename TArgHead, typename ...TArgs>
	static TRet func_caller(mrb_state* mrb, typename currier< std::function< TRet(TArgHead, TArgs...) > >::type fn, mrb_value* args)
	{
		return func_caller<idx+1, TRet, TArgs...>(
			mrb, 
			fn(MRubyTypeBinder<TArgHead>::from_mrb_value(mrb, args[idx]) ), 
			args);
	}


	template<typename TRet, typename ... TArgs>
	static mrb_value mruby_func_caller(mrb_state* mrb, mrb_value self)
	{
		typedef TRet(*func_t)(TArgs...);

		RClass* cls = get_object_from<RClass*>(mrb, self);
		mrb_value* args;
		size_t narg = 1;
		mrb_get_args(mrb, "*", &args, &narg);
		mrb_sym func_ptr_sym = mrb_intern_cstr(mrb, typeid(TRet(TArgs...)).name());
		mrb_value func_ptr_holder = mrb_mod_cv_get(mrb, cls, func_ptr_sym);

		func_t func = (func_t)MRubyTypeBinder<size_t>::from_mrb_value(mrb, func_ptr_holder);

		auto curried = curry(func);
		TRet result = func_caller<0, TRet, TArgs...>(mrb, curried, args);
		return MRubyTypeBinder<TRet>::to_mrb_value(mrb, result);
	}



	template<typename TRet, typename ... TArgs>
	void create_function(std::string name, TRet(*func)(TArgs...))
	{
		const int argcount = sizeof...(TArgs);
		RClass* module_class = mrb->kernel_module;
		if (cls != nullptr)
		{
			module_class = cls;
		}
		mrb_sym func_ptr_sym = mrb_intern_cstr(mrb.get(), typeid(TRet(TArgs...)).name());
		mrb_mod_cv_set(mrb.get(), module_class, func_ptr_sym, MRubyTypeBinder<size_t>::to_mrb_value(mrb.get(), (size_t)func));
		mrb_define_module_function(mrb.get(), module_class, name.c_str(), mruby_func_caller<TRet, TArgs...>, MRB_ARGS_REQ(argcount));
		
	}


};

template<class TClass>
class MRubyClass : public MRubyModule
{
	class MRubyNativeObject
	{
		std::string classname;
		mrb_data_type datatype;
		std::shared_ptr<TClass> instance;

	public:
		MRubyNativeObject(std::string classname, std::shared_ptr<TClass> instance) : 
			classname(classname), 
			instance(instance)
		{
			datatype.dfree = destructor;
			datatype.struct_name = classname.c_str();
		}

		~MRubyNativeObject()
		{
		}

		mrb_data_type* get_type_ptr()
		{
			return &datatype;
		}
	};

	static void destructor(mrb_state* mrb, void* ptr)
	{
		delete (MRubyNativeObject*)ptr;
	}

	static mrb_value constructor(mrb_state* mrb, mrb_value self)
	{
		RClass* cls = get_object_from<RClass*>(mrb, self);

		mrb_sym nsym = mrb_intern_lit(mrb, "__classname__");
		mrb_value nval = mrb_obj_iv_get(mrb, (struct RObject*)cls, nsym);
		std::string str = get_object_from<std::string>(mrb, nval);
		std::shared_ptr<TClass> instance = std::make_shared<TClass>();

		MRubyNativeObject* ptr = new MRubyNativeObject(str, instance);

		auto data = RDATA(self);

		DATA_TYPE(self) = ptr->get_type_ptr();
		DATA_PTR(self) = ptr;

		return self;
	}

public:

	MRubyClass(std::shared_ptr<mrb_state> mrb, std::string name, RClass* cls) :
		MRubyModule(mrb, name, cls)
	{
		MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
		mrb_define_method(mrb.get(), cls, "initialize", constructor, MRB_ARGS_ARG(0,0));
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
