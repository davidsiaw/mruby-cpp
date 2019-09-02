#ifndef __MRUBYCLASS_HPP__
#define __MRUBYCLASS_HPP__

template<class TClass>
class Class : public Module
{
	template <typename ... TConstructorArgs>
	static mrb_value constructor(mrb_state* mrb, mrb_value self)
	{
		RClass* cls = TypeBinder<RClass*>::from_mrb_value(mrb, self);

		mrb_sym nsym = mrb_intern_lit(mrb, "__classname__");
		mrb_value nval = mrb_obj_iv_get(mrb, (struct RObject*)cls, nsym);
		std::string str = TypeBinder<std::string>::from_mrb_value(mrb, nval);

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
			return error_argument_count(mrb, self, TypeBinder<mrb_sym>::to_mrb_value(mrb, mrb_intern_cstr(mrb, "initialize")), argc, sizeof...(TConstructorArgs));
		}

		auto curried = curry(func);
		std::shared_ptr<TClass> instance;
		try
		{
			instance = func_caller<0, std::shared_ptr<TClass>, TConstructorArgs...>(mrb, curried, args);
		}
		catch (const RubyException &e)
		{
			mrb_raise(mrb, E_RUNTIME_ERROR, e.what());
		}

		NativeObject<TClass>* ptr = new NativeObject<TClass>(str, instance);

		DATA_TYPE(self) = ptr->get_type_ptr();
		DATA_PTR(self) = ptr;

		return self;
	}

	static mrb_value default_constructor(mrb_state* mrb, mrb_value self)
	{
		RClass* cls = TypeBinder<RClass*>::from_mrb_value(mrb, self);

		mrb_sym nsym = mrb_intern_lit(mrb, "__classname__");
		mrb_value nval = mrb_obj_iv_get(mrb, (struct RObject*)cls, nsym);
		std::string str = TypeBinder<std::string>::from_mrb_value(mrb, nval);
		
		mrb_value* args;
		size_t argc = 0;
		mrb_get_args(mrb, "*", &args, &argc);

		if (argc != 0)
		{
			return error_argument_count(mrb, self, TypeBinder<mrb_sym>::to_mrb_value(mrb, mrb_intern_cstr(mrb, "initialize")), argc, 0);
		}

		std::shared_ptr<TClass> instance;
		try
		{
			instance = std::make_shared<TClass>();
		}
		catch (const RubyException &e)
		{
			mrb_raise(mrb, E_RUNTIME_ERROR, e.what());
		}

		NativeObject<TClass>* ptr = new NativeObject<TClass>(str, instance);

		DATA_TYPE(self) = ptr->get_type_ptr();
		DATA_PTR(self) = ptr;

		return self;
	}

	static mrb_value error_constructor_closed(mrb_state *mrb, mrb_value class_name) {
		mrb_raisef(mrb, E_ARGUMENT_ERROR, "'%S' cannot be created with new",
			class_name);
		return mrb_nil_value();
	}

	static mrb_value closed_constructor(mrb_state* mrb, mrb_value self)
	{
		return error_constructor_closed(mrb, self);
	}
public:

	template <typename ... TConstructorArgs>
	Class(std::shared_ptr<mrb_state> mrb, const std::string&  name, RClass* cls, void(*)(TConstructorArgs...)) :
		Module(mrb, name, cls)
	{
		MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
		mrb_define_method(mrb.get(), cls, "initialize", constructor<TConstructorArgs...>, MRB_ARGS_ARG(sizeof...(TConstructorArgs),0));
	}

	Class(std::shared_ptr<mrb_state> mrb, const std::string&  name, RClass* cls, void(*)(void)) :
		Module(mrb, name, cls)
	{
		MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
		mrb_define_method(mrb.get(), cls, "initialize", default_constructor, MRB_ARGS_ARG(0, 0));
	}

	Class(std::shared_ptr<mrb_state> mrb, const std::string&  name, RClass* cls) :
		Module(mrb, name, cls)
	{
		MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
		mrb_define_method(mrb.get(), cls, "initialize", closed_constructor, MRB_ARGS_ARG(0, 0));
	}

	~Class()
	{

	}


	template<typename TVal>
	static mrb_value mruby_member_setter(mrb_state* mrb, mrb_value self)
	{
		typedef TVal TClass::* memptr_t;
		RClass* cls = TypeBinder<RClass*>::from_mrb_value(mrb, self);

		mrb_value* args;
		size_t argc = 0;
		mrb_get_args(mrb, "*", &args, &argc);

		mrb_value kernel_val = TypeBinder<RClass*>::to_mrb_value(mrb, mrb->kernel_module);
		mrb_value nval = mrb_funcall(mrb, kernel_val, "__method__", 0);
		std::string name = TypeBinder<std::string>::from_mrb_value(mrb, nval);
		std::string ptr_name = "__allocated_memptr__" + name.substr(0, name.length()-1);

		mrb_sym mem_ptr_sym = mrb_intern_cstr(mrb, ptr_name.c_str());
		mrb_value mem_ptr_holder = mrb_mod_cv_get(mrb, cls, mem_ptr_sym);

		if (argc != 1)
		{
			return error_argument_count(mrb, self, nval, argc, 1);
		}

		memptr_t* ptr = (memptr_t*)TypeBinder<size_t>::from_mrb_value(mrb, mem_ptr_holder);
		NativeObject<TClass>* thisptr = (NativeObject<TClass>*)DATA_PTR(self);

		thisptr->get_instance()->**ptr = TypeBinder<TVal>::from_mrb_value(mrb, *args);

		return mrb_nil_value();
	}

	template<typename TVal>
	static mrb_value mruby_member_getter(mrb_state* mrb, mrb_value self)
	{
		typedef TVal TClass::* memptr_t;
		RClass* cls = TypeBinder<RClass*>::from_mrb_value(mrb, self);
		
		mrb_value kernel_val = TypeBinder<RClass*>::to_mrb_value(mrb, mrb->kernel_module);
		mrb_value nval = mrb_funcall(mrb, kernel_val, "__method__", 0);
		std::string name = TypeBinder<std::string>::from_mrb_value(mrb, nval);
		std::string ptr_name = "__allocated_memptr__" + name;

		mrb_sym mem_ptr_sym = mrb_intern_cstr(mrb, ptr_name.c_str());
		mrb_value mem_ptr_holder = mrb_mod_cv_get(mrb, cls, mem_ptr_sym);

		memptr_t* ptr = (memptr_t*)TypeBinder<size_t>::from_mrb_value(mrb, mem_ptr_holder);
		NativeObject<TClass>* thisptr = (NativeObject<TClass>*)DATA_PTR(self);

		return TypeBinder<TVal>::to_mrb_value(mrb, thisptr->get_instance()->**ptr);
	}

	template<typename TVal>
	void create_accessor(const std::string& name, TVal TClass::* var, RClass* module_class)
	{
		typedef TVal TClass::* memptr_t;
		std::string ptr_name = "__allocated_memptr__" + name;
		mrb_sym mem_ptr_sym = mrb_intern_cstr(mrb.get(), ptr_name.c_str());

		function_definer_t define_function_method = mrb_define_method;

		memptr_t* ptr = new memptr_t;
		*ptr = var;

		mrb_mod_cv_set(
			mrb.get(),
			module_class,
			mem_ptr_sym,
			TypeBinder<size_t>::to_mrb_value(mrb.get(), (size_t)ptr));

		define_function_method(
			mrb.get(),
			module_class,
			name.c_str(),
			mruby_member_getter<TVal>,
			MRB_ARGS_REQ(0));

		define_function_method(
			mrb.get(),
			module_class,
			(name + "=").c_str(),
			mruby_member_setter<TVal>,
			MRB_ARGS_REQ(1));
	}

	template<typename TVar>
	void bind_instance_variable(const std::string& name, TVar TClass::* var)
	{
		create_accessor(name, var, cls);
	}

#ifndef _MSC_VER
	template<typename TC2, typename TVar>
	void bind_instance_variable(const std::string& name, TVar TC2::* var)
	{
		static_assert(std::is_base_of<TC2, TClass>::value, "You must provide a member variable that is somewhere up the inheritance chain of this class");
		create_accessor(name, (TVar TClass::*)(var), cls);
	}
#endif

	template<typename TRet, typename ... TArgs>
	void bind_instance_method(const std::string& name, TRet(TClass::*func)(TArgs...))
	{
		create_function(name, func, cls, mrb_define_method);
	}

	template<typename TRet, typename ... TArgs>
	void bind_instance_method(const std::string& name, TRet(TClass::*func)(TArgs...) const)
	{
		create_function(name, func, cls, mrb_define_method);
	}

#ifndef _MSC_VER
	template<typename TC2, typename TRet, typename ... TArgs>
	void bind_instance_method(const std::string& name, TRet(TC2::*func)(TArgs...))
	{
		static_assert(std::is_base_of<TC2, TClass>::value, "You must provide a member function that is somewhere in the inheritance chain of this class");
		typedef TRet(TClass::*TTarget)(TArgs...);
		create_function(name, (TTarget)func, cls, mrb_define_method);
	}
#endif
};

#endif // __MRUBYCLASS_HPP__
