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
#include <type_traits>
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
	typedef void(*function_definer_t)(mrb_state*, RClass*, const char*, mrb_func_t, mrb_aspec);

	std::shared_ptr<mrb_state> mrb;
	RClass* cls;

	MRubyModule(std::shared_ptr<mrb_state> mrb) :
		MRubyModule(mrb, "", nullptr)
	{

	}

	template<typename TRet, typename ... TArgs>
	void create_function(std::string name, TRet(*func)(TArgs...), RClass* module_class, function_definer_t define_function_method)
	{
		const int argcount = sizeof...(TArgs);
		std::string ptr_name = "__funcptr__" + name;
		mrb_sym func_ptr_sym = mrb_intern_cstr(mrb.get(), ptr_name.c_str());
		mrb_mod_cv_set(
			mrb.get(),
			module_class,
			func_ptr_sym,
			MRubyTypeBinder<size_t>::to_mrb_value(mrb.get(), (size_t)func));
		
		define_function_method(
			mrb.get(),
			module_class,
			name.c_str(),
			mruby_func_caller<TRet, TArgs...>,
			MRB_ARGS_REQ(argcount));

	}

	template<typename TRet, typename TClass, typename ... TArgs>
	void create_function(std::string name, TRet(TClass::*func)(TArgs...), RClass* module_class, function_definer_t define_function_method)
	{
		typedef TRet(TClass::* memfuncptr_t)(TArgs...);
		const int argcount = sizeof...(TArgs);
		std::string ptr_name = "__allocated_funcptr__" + name;
		mrb_sym func_ptr_sym = mrb_intern_cstr(mrb.get(), ptr_name.c_str());

		memfuncptr_t* ptr = new memfuncptr_t;
		*ptr = func;

		mrb_mod_cv_set(
			mrb.get(),
			module_class,
			func_ptr_sym,
			MRubyTypeBinder<size_t>::to_mrb_value(mrb.get(), (size_t)ptr));

		define_function_method(
			mrb.get(),
			module_class,
			name.c_str(),
			mruby_member_func_caller<TRet, TClass, TArgs...>,
			MRB_ARGS_REQ(argcount));

	}

private:
	std::string name;

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
		) {} // : result(
	};

	template <typename TRet, typename ...TArgs>
	static auto curry(const std::function<TRet(TArgs...)> fun)
		-> typename currier< std::function< TRet(TArgs...) > >::type
	{
		return currier< std::function< TRet(TArgs...) > >(fun).result;
	}

	template <typename TRet, typename ...TArgs>
	static auto curry(TRet(*const fun)(TArgs...))
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
	static TRet func_caller(
		mrb_state* mrb,
		typename currier< std::function< TRet(TArgHead, TArgs...) > >::type fn,
		mrb_value* args)
	{
		return func_caller<idx + 1, TRet, TArgs...>(
			mrb,
			fn(MRubyTypeBinder<TArgHead>::from_mrb_value(mrb, args[idx])),
			args);
	}


	template <int idx, typename TArgHead>
	static void void_func_caller(
		mrb_state* mrb,
		typename currier< std::function< void(TArgHead) > >::type fn,
		mrb_value* args)
	{
		fn(MRubyTypeBinder<TArgHead>::from_mrb_value(mrb, args[idx]));
	}

	template <int idx, typename TArgHead, typename TArgHead2, typename ...TArgs>
	static void void_func_caller(
		mrb_state* mrb,
		typename currier< std::function< void(TArgHead, TArgHead2, TArgs...) > >::type fn,
		mrb_value* args)
	{
		void_func_caller<idx + 1, TArgHead2, TArgs...>(
			mrb,
			fn(MRubyTypeBinder<TArgHead>::from_mrb_value(mrb, args[idx])),
			args);
	}


	template<typename TRet, typename ... TArgs>
	struct mruby_func_called_returner
	{
		static mrb_value call(mrb_state* mrb, std::function<TRet(TArgs...)> func, mrb_value* args)
		{
			auto curried = curry(func);
			TRet result = func_caller<0, TRet, TArgs...>(mrb, curried, args);
			return MRubyTypeBinder<TRet>::to_mrb_value(mrb, result);
		}
	};

	template<typename ... TArgs>
	struct mruby_func_called_returner<void, TArgs...>
	{
		static mrb_value call(mrb_state* mrb, std::function<void(TArgs...)> func, mrb_value* args)
		{
			auto curried = curry(func);
			void_func_caller<0, TArgs...>(mrb, curried, args);
			return mrb_nil_value();
		}
	};

	template<typename TRet>
	struct mruby_func_called_returner<TRet>
	{
		static mrb_value call(mrb_state* mrb, std::function<TRet()> func, mrb_value* args)
		{
			TRet result = func();
			return MRubyTypeBinder<TRet>::to_mrb_value(mrb, result);
		}
	};

	template<>
	struct mruby_func_called_returner<void>
	{
		static mrb_value call(mrb_state* mrb, std::function<void()> func, mrb_value* args)
		{
			func();
			return mrb_nil_value();
		}
	};

	mrb_value raise_wrong_arg_count(mrb_state *mrb, mrb_value func_name, int argc, int paramc) {
		mrb_raisef(mrb, E_ARGUMENT_ERROR, "'%S': wrong number of arguments (%S for %S)",
			func_name,
			mrb_fixnum_value(argc),
			mrb_fixnum_value(paramc));
		return mrb_nil_value();
	}

	template< typename TRet, typename ... TArgs >
	static mrb_value mruby_func_caller(mrb_state* mrb, mrb_value self)
	{
		typedef TRet(func_t)(TArgs...);

		RClass* cls = get_object_from<RClass*>(mrb, self);
		mrb_value* args;
		size_t argc = 0;
		mrb_get_args(mrb, "*", &args, &argc);

		mrb_value kernel_val = get_value_from<RClass*>(mrb, mrb->kernel_module);
		mrb_value nval = mrb_funcall(mrb, kernel_val, "__method__", 0);
		std::string name = get_object_from<std::string>(mrb, nval);
		std::string ptr_name = "__funcptr__" + get_object_from<std::string>(mrb, nval);

		mrb_sym func_ptr_sym = mrb_intern_cstr(mrb, ptr_name.c_str());
		mrb_value func_ptr_holder = mrb_mod_cv_get(mrb, cls, func_ptr_sym);

		if (argc != sizeof...(TArgs))
		{

			return mrb_nil_value();
		}

		func_t* func = (func_t*)MRubyTypeBinder<size_t>::from_mrb_value(mrb, func_ptr_holder);
		std::function<func_t> func_obj(func);
		return mruby_func_called_returner<TRet, TArgs...>::call(mrb, func_obj, args);
	}

	template< typename TRet, typename TClass, typename ... TArgs >
	static mrb_value mruby_member_func_caller(mrb_state* mrb, mrb_value self)
	{
		typedef TRet(TClass::*memfuncptr_t)(TArgs...);
		typedef TRet(*func_t)(TClass*, TArgs...);

		RClass* cls = get_object_from<RClass*>(mrb, self);

		mrb_value* args;
		size_t argc = 0;
		mrb_get_args(mrb, "*", &args, &argc);

		mrb_value kernel_val = get_value_from<RClass*>(mrb, mrb->kernel_module);
		mrb_value nval = mrb_funcall(mrb, kernel_val, "__method__", 0);
		std::string name = get_object_from<std::string>(mrb, nval);
		std::string ptr_name = "__allocated_funcptr__" + get_object_from<std::string>(mrb, nval);

		mrb_sym func_ptr_sym = mrb_intern_cstr(mrb, ptr_name.c_str());
		mrb_value func_ptr_holder = mrb_mod_cv_get(mrb, cls, func_ptr_sym);

		if (argc != sizeof...(TArgs))
		{

			return mrb_nil_value();
		}

		memfuncptr_t* ptr = (memfuncptr_t*)MRubyTypeBinder<size_t>::from_mrb_value(mrb, func_ptr_holder);
		memfuncptr_t func = *ptr;
		TClass* thisptr = (TClass*)DATA_PTR(self);

		auto callable = std::bind(func, thisptr);
		
		return mruby_func_called_returner<TRet, TArgs...>::call(
			mrb, 
			[=](TArgs... params)->TRet 
			{
				return callable(params...);
			}, 
			args);
	}

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

	template<typename TRet, typename ... TArgs>
	void bind_method(std::string name, TRet(*func)(TArgs...))
	{
		if (cls == nullptr)
		{
			create_function(name, func, mrb->kernel_module, mrb_define_module_function);
		}
		else if (cls->tt == MRB_TT_MODULE)
		{
			create_function(name, func, cls, mrb_define_module_function);
		}
		else if (cls->tt == MRB_TT_CLASS)
		{
			create_function(name, func, cls, mrb_define_class_method);
		}
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

	template<typename TRet, typename TClass, typename ... TArgs>
	void bind_instance_method(std::string name, TRet(TClass::*func)(TArgs...))
	{
		create_function(name, func, cls, mrb_define_method);
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
