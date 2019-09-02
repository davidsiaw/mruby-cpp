#ifndef __MRUBYFUNCTION_HPP__
#define __MRUBYFUNCTION_HPP__

class BaseFunction
{
protected:
	mrb_state* mrb;
	std::shared_ptr<RProc> proc;
	BaseFunction(mrb_state* mrb, std::shared_ptr<RProc> proc) : mrb(mrb), proc(proc)
	{ }

	std::shared_ptr<RProc> not_owned(RProc* proc)
	{
		return std::shared_ptr<RProc>(proc, [](RProc*) {});
	}

	template<typename TArgHead, typename TArgHead2, typename ... TArgTail>
	static void push_args(mrb_state* mrb, std::vector<mrb_value>* vector, TArgHead head, TArgHead2 head2, TArgTail ... tail)
	{
		vector->push_back(TypeBinder<TArgHead>::to_mrb_value(mrb, head));
		push_args<TArgHead2, TArgTail...>(mrb, vector, head2, tail...);
	}

	template<typename TArgHead>
	static void push_args(mrb_state* mrb, std::vector<mrb_value>* vector, TArgHead head)
	{
		vector->push_back(TypeBinder<TArgHead>::to_mrb_value(mrb, head));
	}
	
public:
	std::shared_ptr<RProc> get_proc() const
	{
		return proc;
	}
};

template<typename TFunc>
class Function : public BaseFunction
{
	Function()
	{ /* private constructor */
	}
};

template<typename TRet, typename ... TArgs>
class Function<TRet(TArgs...)> : public BaseFunction
{
public:
	Function(mrb_state* mrb, RProc* proc) : BaseFunction(mrb, not_owned(proc))
	{ }

	TRet invoke(TArgs... args)
	{
		size_t argc = sizeof...(TArgs);
		std::vector<mrb_value> argvector;
		push_args(mrb, &argvector, args...);

		mrb_sym call = mrb_intern_cstr(mrb, "call");
		return TypeBinder<TRet>::from_mrb_value(
			mrb,
			mrb_funcall_argv(
				mrb,
				TypeBinder<RProc*>::to_mrb_value(mrb, proc.get()),
				call,
				argc,
				&argvector[0]));
	}
};

template<typename ... TArgs>
class Function<void(TArgs...)> : public BaseFunction
{
public:
	Function(mrb_state* mrb, RProc* proc) : BaseFunction(mrb, not_owned(proc))
	{ }

	void invoke(TArgs... args)
	{
		size_t argc = sizeof...(TArgs);
		std::vector<mrb_value> argvector;
		push_args(mrb, &argvector, args...);

		mrb_sym call = mrb_intern_cstr(mrb, "call");
		mrb_funcall_argv(
			mrb,
			TypeBinder<RProc*>::to_mrb_value(mrb, proc.get()),
			call,
			argc,
			&argvector[0]);
	}
};

template<typename TRet>
class Function<TRet()> : public BaseFunction
{
public:
	Function(mrb_state* mrb, RProc* proc) : BaseFunction(mrb, not_owned(proc))
	{ }

	TRet invoke()
	{
		mrb_value no = mrb_nil_value();

		mrb_sym call = mrb_intern_cstr(mrb, "call");
		return TypeBinder<TRet>::from_mrb_value(
			mrb,
			mrb_funcall_argv(mrb, TypeBinder<RProc*>::to_mrb_value(mrb, proc), call, 0, &no));
	}
};

template<>
class Function< void() > : public BaseFunction
{
	using func_wrapper_t = NativeObject< std::function< void() > >;

	static mrb_value cfunc_for_mrb(mrb_state* mrb, mrb_value self)
	{
		mrb_value boxed_obj_wrap = mrb_proc_cfunc_env_get(mrb, 0);
		auto obj_wrap = TypeBinder<func_wrapper_t>::from_mrb_value(mrb, boxed_obj_wrap);
		obj_wrap.get_instance()->operator()();
		return mrb_nil_value();
	}

	std::shared_ptr<RProc> make_proc(std::function< void() > func)
	{
		func_wrapper_t obj_wrap("Object", std::make_shared< std::function<void()> >(func));
		mrb_value boxed_obj_wrap = TypeBinder<func_wrapper_t>::to_mrb_value(mrb, obj_wrap);
		RProc* proc_ptr = mrb_proc_new_cfunc_with_env(mrb, cfunc_for_mrb, 1, &boxed_obj_wrap);
		return not_owned(proc_ptr);
	}

public:
	Function(mrb_state* mrb, RProc* proc) : BaseFunction(mrb, not_owned(proc))
	{ }

	Function(mrb_state* mrb, std::function< void() > func) : BaseFunction(mrb, make_proc(func))
	{ }

	void invoke()
	{
		mrb_value no = mrb_nil_value();

		mrb_sym call = mrb_intern_cstr(mrb, "call");
		mrb_funcall_argv(mrb, TypeBinder<RProc*>::to_mrb_value(mrb, proc.get()), call, 0, &no);
	}
};
#endif // __MRUBYFUNCTION_HPP__
