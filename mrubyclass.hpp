#ifndef __MRUBYCLASS_HPP__
#define __MRUBYCLASS_HPP__

template<class TClass>
class Class : public Module
{
	static void destructor(mrb_state* mrb, void* ptr)
	{
		delete (NativeObject<TClass>*)ptr;
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
			return error_argument_count(mrb, self, TypeBinder<mrb_sym>::to_mrb_value(mrb, mrb_intern_cstr(mrb, "initialize")), argc, sizeof...(TConstructorArgs));
		}

		auto curried = curry(func);
		std::shared_ptr<TClass> instance = func_caller<0, std::shared_ptr<TClass>, TConstructorArgs...>(mrb, curried, args);

		NativeObject<TClass>* ptr = new NativeObject<TClass>(str, instance, &destructor);

		auto data = RDATA(self);

		DATA_TYPE(self) = ptr->get_type_ptr();
		DATA_PTR(self) = ptr;

		return self;
	}

	static mrb_value default_constructor(mrb_state* mrb, mrb_value self)
	{
		RClass* cls = get_object_from<RClass*>(mrb, self);

		mrb_sym nsym = mrb_intern_lit(mrb, "__classname__");
		mrb_value nval = mrb_obj_iv_get(mrb, (struct RObject*)cls, nsym);
		std::string str = get_object_from<std::string>(mrb, nval);
		
		mrb_value* args;
		size_t argc = 0;
		mrb_get_args(mrb, "*", &args, &argc);

		if (argc != 0)
		{
			return error_argument_count(mrb, self, TypeBinder<mrb_sym>::to_mrb_value(mrb, mrb_intern_cstr(mrb, "initialize")), argc, 0);
		}

		std::shared_ptr<TClass> instance = std::make_shared<TClass>();

		NativeObject<TClass>* ptr = new NativeObject<TClass>(str, instance, &destructor);

		auto data = RDATA(self);

		DATA_TYPE(self) = ptr->get_type_ptr();
		DATA_PTR(self) = ptr;

		return self;
	}

public:

	template <typename ... TConstructorArgs>
	Class(std::shared_ptr<mrb_state> mrb, const std::string&  name, RClass* cls, void(*)(TConstructorArgs...)) :
		Module(mrb, name, cls)
	{
		MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
		mrb_define_method(mrb.get(), cls, "initialize", constructor<TConstructorArgs...>, MRB_ARGS_ARG(sizeof...(TConstructorArgs),0));
	}

	template <>
	Class(std::shared_ptr<mrb_state> mrb, const std::string&  name, RClass* cls, void(*)()) :
		Module(mrb, name, cls)
	{
		MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
		mrb_define_method(mrb.get(), cls, "initialize", default_constructor, MRB_ARGS_ARG(0, 0));
	}

	~Class()
	{

	}

	template<typename TRet, typename ... TArgs>
	void bind_instance_method(const std::string& name, TRet(TClass::*func)(TArgs...))
	{
		create_function(name, func, cls, mrb_define_method);
	}
};

#endif // __MRUBYCLASS_HPP__
