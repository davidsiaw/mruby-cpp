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
		std::shared_ptr<TClass> instance = func_caller<0, std::shared_ptr<TClass>, TConstructorArgs...>(mrb, curried, args);

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

		std::shared_ptr<TClass> instance = std::make_shared<TClass>();

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

	template<typename TRet, typename ... TArgs>
	void bind_instance_method(const std::string& name, TRet(TClass::*func)(TArgs...))
	{
		create_function(name, func, cls, mrb_define_method);
	}

};



#endif // __MRUBYCLASS_HPP__
