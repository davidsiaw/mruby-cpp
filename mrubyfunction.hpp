#ifndef __MRUBYFUNCTION_HPP__
#define __MRUBYFUNCTION_HPP__

class BaseFunction
{
protected:
	mrb_state* mrb;
	RProc* proc;
	BaseFunction(mrb_state* mrb, RProc* proc) : mrb(mrb), proc(proc)
	{ }

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
	Function(mrb_state* mrb, RProc* proc) : BaseFunction(mrb, proc)
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
				TypeBinder<RProc*>::to_mrb_value(mrb, proc),
				call,
				argc,
				&argvector[0]));
	}
};

template<typename ... TArgs>
class Function<void(TArgs...)> : public BaseFunction
{
public:
	Function(mrb_state* mrb, RProc* proc) : BaseFunction(mrb, proc)
	{ }

	void invoke(TArgs... args)
	{
		size_t argc = sizeof...(TArgs);
		std::vector<mrb_value> argvector;
		push_args(mrb, &argvector, args...);

		mrb_sym call = mrb_intern_cstr(mrb, "call");
		mrb_funcall_argv(
			mrb,
			TypeBinder<RProc*>::to_mrb_value(mrb, proc),
			call,
			argc,
			&argvector[0]);
	}
};

template<typename TRet>
class Function<TRet()> : public BaseFunction
{
public:
	Function(mrb_state* mrb, RProc* proc) : BaseFunction(mrb, proc)
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
public:
	Function(mrb_state* mrb, RProc* proc) : BaseFunction(mrb, proc)
	{ }

	void invoke()
	{
		mrb_value no = mrb_nil_value();

		mrb_sym call = mrb_intern_cstr(mrb, "call");
		mrb_funcall_argv(mrb, TypeBinder<RProc*>::to_mrb_value(mrb, proc), call, 0, &no);
	}
};
#endif // __MRUBYFUNCTION_HPP__
