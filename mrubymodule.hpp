#ifndef __MRUBYMODULE_HPP__
#define __MRUBYMODULE_HPP__

template<class TClass>
class Class;

class Module
{
protected:
	typedef void(*function_definer_t)(mrb_state*, RClass*, const char*, mrb_func_t, mrb_aspec);

	std::shared_ptr<mrb_state> mrb;
	RClass* cls;

	Module(std::shared_ptr<mrb_state> mrb) :
		Module(mrb, "", nullptr)
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

		func_t* func = (func_t*)TypeBinder<size_t>::from_mrb_value(mrb, func_ptr_holder);
		std::function<func_t> func_obj(func);
		return mruby_func_called_returner<TRet, TArgs...>::call(mrb, func_obj, args);
	}

	template< typename TRet, typename TClass, typename ... TArgs >
	static mrb_value mruby_member_func_caller(mrb_state* mrb, mrb_value self)
	{
		typedef TRet(TClass::*memfuncptr_t)(TArgs...);
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

		memfuncptr_t* ptr = (memfuncptr_t*)TypeBinder<size_t>::from_mrb_value(mrb, func_ptr_holder);
		memfuncptr_t func = *ptr;
		NativeObject<TClass>* thisptr = (NativeObject<TClass>*)DATA_PTR(self);

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
	void create_function(std::string name, TRet(*func)(TArgs...), RClass* module_class, function_definer_t define_function_method)
	{
		const int argcount = sizeof...(TArgs);
		std::string ptr_name = "__funcptr__" + name;
		mrb_sym func_ptr_sym = mrb_intern_cstr(mrb.get(), ptr_name.c_str());
		mrb_mod_cv_set(
			mrb.get(),
			module_class,
			func_ptr_sym,
			TypeBinder<size_t>::to_mrb_value(mrb.get(), (size_t)func));
		
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
			TypeBinder<size_t>::to_mrb_value(mrb.get(), (size_t)ptr));

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
	Module(std::shared_ptr<mrb_state> mrb, std::string name, RClass* cls) :
		mrb(mrb),
		cls(cls), 
		name(name)
	{

	}

	~Module()
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

	std::shared_ptr<Module> get_class(std::string name)
	{
		if (!thing_is_defined(name, MRB_TT_CLASS))
		{
			throw Exception("Module does not exist", name);
		}
		RClass *theclass = NULL;
		return std::make_shared<Module>(mrb, name, theclass);
	}

	std::shared_ptr<Module> get_module(std::string name)
	{
		if (!thing_is_defined(name, MRB_TT_MODULE))
		{
			throw Exception("Module does not exist", name);
		}
		RClass *theclass = NULL;
		return std::make_shared<Module>(mrb, name, theclass);
	}

	std::shared_ptr<Module> create_module(std::string name)
	{
		if (cls == nullptr)
		{
			RClass* topmodule = mrb_define_module(mrb.get(), name.c_str());
			return std::make_shared<Module>(mrb, name, topmodule);
		}

		RClass* submodule = mrb_define_module_under(mrb.get(), cls, name.c_str());
		return std::make_shared<Module>(mrb, name, submodule);
	}

	static constexpr void* DUMMY_VALUE_TO_PASS_TYPE_TO_CONSTRUCTOR = nullptr;

	template<typename TClass, typename ... TConstructorArgs>
	std::shared_ptr< Class<TClass> > create_class(std::string name)
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

		return std::make_shared<Class<TClass>>(mrb, name, rubyclass, (typepasser_t)DUMMY_VALUE_TO_PASS_TYPE_TO_CONSTRUCTOR);
	}

	template<typename T>
	void set_class_variable(std::string name, T value)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			mrb_vm_iv_set(mrb.get(), var_name_sym, TypeBinder<T>::to_mrb_value(mrb.get(), value));
		}
		else
		{
			mrb_iv_set(mrb.get(), mrb_obj_value(cls), var_name_sym, TypeBinder<T>::to_mrb_value(mrb.get(), value));
		}
	}

	template<typename T>
	T get_class_variable(std::string name)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			return TypeBinder<T>::from_mrb_value(mrb.get(),  mrb_vm_iv_get(mrb.get(), var_name_sym));
		}
		else
		{
			return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_iv_get(mrb.get(), mrb_obj_value(cls), var_name_sym));
		}
	}

	template<typename T>
	void set_global_variable(std::string name, T value)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		mrb_gv_set(mrb.get(), var_name_sym, TypeBinder<T>::to_mrb_value(mrb.get(), value));
	}

	template<typename T>
	T get_global_variable(std::string name)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_gv_get(mrb.get(), var_name_sym));
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

#endif // __MRUBYMODULE_HPP__
