#ifndef __MRUBY_HPP__
#define __MRUBY_HPP__

#include <memory>
#include <functional>
#include <sstream>
#include <type_traits>
#include <fstream>

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <mruby/variable.h>
#include <mruby/string.h>

class MRubyException
{
public:
	MRubyException(const std::string&, const std::string&)
	{}
};

template<class TClass>
class MRubyNativeObject
{
	std::string classname;
	mrb_data_type datatype;
	std::shared_ptr<TClass> instance;

public:
	MRubyNativeObject(const std::string& classname, std::shared_ptr<TClass> instance, void(*destructor)(mrb_state*, void*) ) :
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

	TClass* get_instance() const
	{
		return instance.get();
	}
};

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
class MRubyTypeBinder<mrb_sym> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, mrb_sym sym) { return mrb_sym2str(mrb, sym); }
	static mrb_sym from_mrb_value(mrb_state* mrb, mrb_value val) { return mrb_intern_str(mrb, val); }
};

template<>
class MRubyTypeBinder<std::string> {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, const std::string& str) { return mrb_str_new(mrb, str.c_str(), str.size()); }
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
	static mrb_value to_mrb_value(mrb_state* mrb, const std::string& str) { return MRubyTypeBinder<std::string>::to_mrb_value(mrb, str); }
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

template<class TClass>
class MRubyTypeBinder< std::shared_ptr<TClass> > {
public:
	static mrb_value to_mrb_value(mrb_state* mrb, std::shared_ptr<TClass> str)
	{

	}

	static std::shared_ptr<TClass> from_mrb_value(mrb_state* mrb, mrb_value val)
	{
		if (val.tt == MRB_TT_DATA)
		{
			MRubyNativeObject<TClass>* thisptr = (MRubyNativeObject<TClass>*)DATA_PTR(val);
			return thisptr->instance();
		}

		throw MRubyException("Not a data type", "");
	}
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

	static mrb_value error_argument_count(mrb_state *mrb, mrb_value class_name, mrb_value func_name, size_t given, size_t expected) {
		mrb_raisef(mrb, E_ARGUMENT_ERROR, "in '%S': %S: wrong number of arguments (%S for %S)",
			class_name,
			func_name,
			mrb_fixnum_value(given),
			mrb_fixnum_value(expected));
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
			return error_argument_count(mrb, self, nval, argc, sizeof...(TArgs));
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
			return error_argument_count(mrb, self, nval, argc, sizeof...(TArgs));
		}

		memfuncptr_t* ptr = (memfuncptr_t*)MRubyTypeBinder<size_t>::from_mrb_value(mrb, func_ptr_holder);
		memfuncptr_t func = *ptr;
		MRubyNativeObject<TClass>* thisptr = (MRubyNativeObject<TClass>*)DATA_PTR(self);

		auto callable = std::bind(func, thisptr->get_instance());

		return mruby_func_called_returner<TRet, TArgs...>::call(
			mrb,
			[=](TArgs... params)->TRet
		{
			return callable(params...);
		},
			args);
	}

	template<typename TRet, typename ... TArgs>
	void create_function(const std::string& name, TRet(*func)(TArgs...), RClass* module_class, function_definer_t define_function_method)
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
	void create_function(const std::string& name, TRet(TClass::*func)(TArgs...), RClass* module_class, function_definer_t define_function_method)
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


public:
	MRubyModule(std::shared_ptr<mrb_state> mrb, const std::string& name, RClass* cls) :
		mrb(mrb),
		cls(cls), 
		name(name)
	{

	}

	~MRubyModule()
	{

	}

	bool thing_is_defined(const std::string& name, mrb_vtype type)
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

	std::shared_ptr<MRubyModule> get_class(const std::string& name)
	{
		if (!thing_is_defined(name, MRB_TT_CLASS))
		{
			throw MRubyException("Module does not exist", name);
		}
		RClass *theclass = NULL;
		mrb_sym name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		mrb_value m = mrb_vm_const_get(mrb.get(), name_sym);
		return std::make_shared<MRubyModule>(mrb, name, theclass);
	}

	std::shared_ptr<MRubyModule> get_module(const std::string& name)
	{
		if (!thing_is_defined(name, MRB_TT_MODULE))
		{
			throw MRubyException("Module does not exist", name);
		}
		RClass *theclass = NULL;
		mrb_sym name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		mrb_value m = mrb_vm_const_get(mrb.get(), name_sym);
		return std::make_shared<MRubyModule>(mrb, name, theclass);
	}

	std::shared_ptr<MRubyModule> create_module(const std::string& name)
	{
		if (cls == nullptr)
		{
			RClass* topmodule = mrb_define_module(mrb.get(), name.c_str());
			return std::make_shared<MRubyModule>(mrb, name, topmodule);
		}

		RClass* submodule = mrb_define_module_under(mrb.get(), cls, name.c_str());
		return std::make_shared<MRubyModule>(mrb, name, submodule);
	}

	static constexpr void* DUMMY_VALUE_TO_PASS_TYPE_TO_CONSTRUCTOR = nullptr;

	template<typename TClass, typename ... TConstructorArgs>
	std::shared_ptr< MRubyClass<TClass> > create_class(const std::string& name)
	{
		typedef void(*typepasser_t)(TConstructorArgs...);

		RClass* rubyclass = nullptr;
		if (cls == nullptr)
		{
			rubyclass = mrb_define_class(mrb.get(), name.c_str(), mrb->object_class);
		}
		else
		{
			rubyclass = mrb_define_class_under(mrb.get(), cls, name.c_str(), mrb->object_class);
		}

		return std::make_shared<MRubyClass<TClass>>(mrb, name, rubyclass, (typepasser_t)DUMMY_VALUE_TO_PASS_TYPE_TO_CONSTRUCTOR);
	}

	template<typename T>
	void set_class_variable(const std::string& name, T value)
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
	T get_class_variable(const std::string& name)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			return MRubyTypeBinder<T>::from_mrb_value(mrb.get(),  mrb_vm_iv_get(mrb.get(), var_name_sym));
		}
		else
		{
			return MRubyTypeBinder<T>::from_mrb_value(mrb.get(), mrb_iv_get(mrb.get(), mrb_obj_value(cls), var_name_sym));
		}
	}

	template<typename T>
	void set_global_variable(const std::string& name, T value)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		mrb_gv_set(mrb.get(), var_name_sym, MRubyTypeBinder<T>::to_mrb_value(mrb.get(), value));
	}

	template<typename T>
	T get_global_variable(const std::string& name)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		return MRubyTypeBinder<T>::from_mrb_value(mrb.get(), mrb_gv_get(mrb.get(), var_name_sym));
	}

	template<typename TRet, typename ... TArgs>
	void bind_method(const std::string& name, TRet(*func)(TArgs...))
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

	static void destructor(mrb_state* mrb, void* ptr)
	{
		delete (MRubyNativeObject<TClass>*)ptr;
	}

	template <typename ... TConstructorArgs>
	static mrb_value constructor(mrb_state* mrb, mrb_value self)
	{
		RClass* cls = get_object_from<RClass*>(mrb, self);

		mrb_sym nsym = mrb_intern_lit(mrb, "__classname__");
		mrb_value nval = mrb_obj_iv_get(mrb, (struct RObject*)cls, nsym);
		std::string str = get_object_from<std::string>(mrb, nval);

		std::function<std::shared_ptr<TClass>(TConstructorArgs...)> func = 
			[=](TConstructorArgs... params) -> std::shared_ptr<TClass>
			{
				return std::make_shared<TClass>(params...);
			};


		mrb_value* args;
		size_t argc = 0;
		mrb_get_args(mrb, "*", &args, &argc);

		if (argc != sizeof...(TConstructorArgs))
		{
			return error_argument_count(mrb, self, MRubyTypeBinder<mrb_sym>::to_mrb_value(mrb, mrb_intern_cstr(mrb, "initialize")), argc, sizeof...(TConstructorArgs));
		}

		auto curried = curry(func);
		std::shared_ptr<TClass> instance = func_caller<0, std::shared_ptr<TClass>, TConstructorArgs...>(mrb, curried, args);

		MRubyNativeObject<TClass>* ptr = new MRubyNativeObject<TClass>(str, instance, &destructor);

		auto data = RDATA(self);

		DATA_TYPE(self) = ptr->get_type_ptr();
		DATA_PTR(self) = ptr;

		return self;
	}

public:

	template <typename ... TConstructorArgs>
	MRubyClass(std::shared_ptr<mrb_state> mrb, const std::string& name, RClass* cls, void(*)(TConstructorArgs...)) :
		MRubyModule(mrb, name, cls)
	{
		MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
		mrb_define_method(mrb.get(), cls, "initialize", constructor<TConstructorArgs...>, MRB_ARGS_ARG(sizeof...(TConstructorArgs),0));
	}

	~MRubyClass()
	{

	}

	template<typename TRet, typename ... TArgs>
	void bind_instance_method(const std::string& name, TRet(TClass::*func)(TArgs...))
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

#endif // __MRUBY_HPP__