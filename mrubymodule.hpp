#ifndef __MRUBYMODULE_HPP__
#define __MRUBYMODULE_HPP__

#define START_HANDLE_ERROR \
		mrb_value exc = mrb_nil_value(); \
		try // Try cover to cleanup before raising into mruby

#define HANDLE_ERROR_AND_EXIT throw

#define END_HANDLE_ERROR \
		catch(mrb_value the_exception) \
		{ \
			exc = the_exception; \
		} \
		mrb_exc_raise(mrb, exc); \
		return mrb_nil_value();	// This should never run because raise jumps away

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

	static mrb_value error_argument_count(mrb_state *mrb, const std::string &class_name, const std::string &func_name, size_t given, size_t expected) {
		std::stringstream ss;
		ss << "in '" << class_name << "': " << func_name << ": wrong number of arguments (" << given << " for " << expected << ")";

		printf("%s\n", ss.str().c_str());
		return mrb_exc_new(mrb, E_ARGUMENT_ERROR, ss.str().c_str(), ss.str().length());
	}

	template< typename TRet, typename ... TArgs >
	static mrb_value mruby_func_caller(mrb_state* mrb, mrb_value self)
	{
		START_HANDLE_ERROR
		{
			typedef TRet(func_t)(TArgs...);

			RClass* cls = TypeBinder<RClass*>::from_mrb_value(mrb, self);
			mrb_value* args;
			size_t argc = 0;
			mrb_get_args(mrb, "*", &args, &argc);

			mrb_value kernel_val = TypeBinder<RClass*>::to_mrb_value(mrb, mrb->kernel_module);
			mrb_value nval = mrb_funcall(mrb, kernel_val, "__method__", 0);
			std::string name = TypeBinder<std::string>::from_mrb_value(mrb, nval);
			std::string ptr_name = "__funcptr__" + name;

			mrb_sym func_ptr_sym = mrb_intern_cstr(mrb, ptr_name.c_str());
			mrb_value func_ptr_holder = mrb_mod_cv_get(mrb, cls, func_ptr_sym);

			if (argc != sizeof...(TArgs))
			{
				std::string class_name = TypeBinder<std::string>::from_mrb_value(mrb, mrb_inspect(mrb, self));
				HANDLE_ERROR_AND_EXIT error_argument_count(mrb, class_name, name, argc, sizeof...(TArgs));
			}

			func_t* func = (func_t*)TypeBinder<size_t>::from_mrb_value(mrb, func_ptr_holder);
			std::function<func_t> func_obj(func);
			try
			{
				return mruby_func_called_returner<TRet, TArgs...>::call(mrb, func_obj, args);
			}
			catch (const RubyException &e)
			{
				HANDLE_ERROR_AND_EXIT mrb_exc_new(mrb, E_RUNTIME_ERROR, e.what(), strlen(e.what()));
			}
		}
		END_HANDLE_ERROR
	}


	template< typename TRet, typename TClass, typename ... TArgs >
	static mrb_value mruby_member_func_caller(mrb_state* mrb, mrb_value self)
	{
		START_HANDLE_ERROR
		{
			typedef TRet(TClass::*memfuncptr_t)(TArgs...);
			RClass* cls = TypeBinder<RClass*>::from_mrb_value(mrb, self);

			mrb_value* args;
			size_t argc = 0;
			mrb_get_args(mrb, "*", &args, &argc);

			mrb_value kernel_val = TypeBinder<RClass*>::to_mrb_value(mrb, mrb->kernel_module);
			mrb_value nval = mrb_funcall(mrb, kernel_val, "__method__", 0);
			std::string name = TypeBinder<std::string>::from_mrb_value(mrb, nval);
			std::string ptr_name = "__allocated_funcptr__" + name;

			mrb_sym func_ptr_sym = mrb_intern_cstr(mrb, ptr_name.c_str());
			mrb_value func_ptr_holder = mrb_mod_cv_get(mrb, cls, func_ptr_sym);

			if (argc != sizeof...(TArgs))
			{
				std::string class_name = TypeBinder<std::string>::from_mrb_value(mrb, mrb_inspect(mrb, self));
				HANDLE_ERROR_AND_EXIT error_argument_count(mrb, class_name, name, argc, sizeof...(TArgs));
			}

			NativeObject<memfuncptr_t> obj = TypeBinder< NativeObject<memfuncptr_t> >::from_mrb_value(mrb, func_ptr_holder);

			memfuncptr_t* ptr = obj.get_instance();
			NativeObject<TClass>* thisptr = (NativeObject<TClass>*)DATA_PTR(self);

			try
			{
				auto callable = [&](TArgs... params) -> TRet
				{
					return (thisptr->get_instance()->**ptr)(params...);
				};

				return mruby_func_called_returner<TRet, TArgs...>::call(
					mrb,
					[=](TArgs... params)->TRet
					{
						return callable(params...);
					},
					args);
			}
			catch (const RubyException &e)
			{
				HANDLE_ERROR_AND_EXIT mrb_exc_new(mrb, E_RUNTIME_ERROR, e.what(), strlen(e.what()));
			}
		}
		END_HANDLE_ERROR
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
			TypeBinder<size_t>::to_mrb_value(mrb.get(), (size_t)func));
		
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
		NativeObject<memfuncptr_t> obj("Object", std::shared_ptr<memfuncptr_t>(ptr));

		mrb_mod_cv_set(
			mrb.get(),
			module_class,
			func_ptr_sym,
			TypeBinder< NativeObject<memfuncptr_t> >::to_mrb_value(mrb.get(), obj));

		define_function_method(
			mrb.get(),
			module_class,
			name.c_str(),
			mruby_member_func_caller<TRet, TClass, TArgs...>,
			MRB_ARGS_REQ(argcount));

	}

	template<typename TRet, typename TClass, typename ... TArgs>
	void create_function(const std::string& name, TRet(TClass::*func)(TArgs...) const, RClass* module_class, function_definer_t define_function_method)
	{
		typedef TRet(TClass::* memfuncptr_t)(TArgs...) const;
		const int argcount = sizeof...(TArgs);
		std::string ptr_name = "__allocated_funcptr__" + name;
		mrb_sym func_ptr_sym = mrb_intern_cstr(mrb.get(), ptr_name.c_str());

		memfuncptr_t* ptr = new memfuncptr_t;
		*ptr = func;
		NativeObject<memfuncptr_t> obj("Object", std::shared_ptr<memfuncptr_t>(ptr));

		mrb_mod_cv_set(
			mrb.get(),
			module_class,
			func_ptr_sym,
			TypeBinder< NativeObject<memfuncptr_t> >::to_mrb_value(mrb.get(), obj));

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
	Module(std::shared_ptr<mrb_state> mrb, const std::string& name, RClass* cls) :
		mrb(mrb),
		cls(cls), 
		name(name)
	{

	}

	virtual ~Module()
	{

	}

	mrb_sym symbol(const std::string& str)
	{
		return mrb_intern_cstr(mrb.get(), str.c_str());
	}

	bool thing_is_defined(const std::string& name, mrb_vtype type)
	{
		mrb_sym name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			mrb_value val = mrb_vm_const_get(mrb.get(), name_sym);
			return mrb_type(val) == type;
		}
		bool is_defined = mrb_const_defined(mrb.get(), mrb_obj_value(cls), name_sym) == 1;
		if (is_defined)
		{
			mrb_value val = mrb_const_get(mrb.get(), mrb_obj_value(cls), name_sym);
			return mrb_type(val) == type;
		}
		return false;
	}

	std::shared_ptr<Module> get_class(const std::string& name)
	{
		if (!thing_is_defined(name, MRB_TT_CLASS))
		{
			throw NameError("Class does not exist", name);
		}
		RClass *theclass = NULL;
		return std::make_shared<Module>(mrb, name, theclass);
	}

	std::shared_ptr<Module> get_module(const std::string& name)
	{
		if (!thing_is_defined(name, MRB_TT_MODULE))
		{
			throw NameError("Module does not exist", name);
		}
		RClass *theclass = NULL;
		return std::make_shared<Module>(mrb, name, theclass);
	}

	std::shared_ptr<Module> create_module(const std::string& name)
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
	std::shared_ptr< Class<TClass> > create_class(const std::string& name)
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

	template<typename TClass>
	std::shared_ptr< Class<TClass> > create_closed_class(const std::string& name)
	{
		RClass* rubyclass = nullptr;
		if (cls == nullptr)
		{
			rubyclass = mrb_define_class(mrb.get(), name.c_str(), mrb->object_class);
		}
		else
		{
			rubyclass = mrb_define_class_under(mrb.get(), cls, name.c_str(), mrb->object_class);
		}

		return std::make_shared<Class<TClass>>(mrb, name, rubyclass);
	}

	template<typename T>
	void set_class_variable(const std::string& name, T value)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			mrb_cv_set(mrb.get(), mrb_obj_value(mrb->object_class), var_name_sym, TypeBinder<T>::to_mrb_value(mrb.get(), value));
		}
		else
		{
			mrb_cv_set(mrb.get(), mrb_obj_value(cls), var_name_sym, TypeBinder<T>::to_mrb_value(mrb.get(), value));
		}
	}

	template<typename T>
	T get_class_variable(const std::string& name)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_cv_get(mrb.get(), mrb_obj_value(mrb->object_class), var_name_sym));
		}
		else
		{
			return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_cv_get(mrb.get(), mrb_obj_value(cls), var_name_sym));
		}
	}

	template<typename T>
	void set_instance_variable(const std::string& name, T value)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			mrb_iv_set(mrb.get(), mrb_obj_value(mrb->top_self), var_name_sym, TypeBinder<T>::to_mrb_value(mrb.get(), value));
		}
		else
		{
			mrb_iv_set(mrb.get(), mrb_obj_value(cls), var_name_sym, TypeBinder<T>::to_mrb_value(mrb.get(), value));
		}
	}

	template<typename T>
	T get_instance_variable(const std::string& name)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		if (cls == nullptr)
		{
			return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_iv_get(mrb.get(), mrb_obj_value(mrb->top_self), var_name_sym));
		}
		else
		{
			return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_iv_get(mrb.get(), mrb_obj_value(cls), var_name_sym));
		}
	}

	template<typename T>
	void set_global_variable(const std::string& name, T value)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		mrb_gv_set(mrb.get(), var_name_sym, TypeBinder<T>::to_mrb_value(mrb.get(), value));
	}

	template<typename T>
	T get_global_variable(const std::string& name)
	{
		mrb_sym var_name_sym = mrb_intern_cstr(mrb.get(), name.c_str());
		return TypeBinder<T>::from_mrb_value(mrb.get(), mrb_gv_get(mrb.get(), var_name_sym));
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

#endif // __MRUBYMODULE_HPP__
