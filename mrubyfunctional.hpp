#ifndef __MRUBYFUNCTIONAL_HPP__
#define __MRUBYFUNCTIONAL_HPP__


template<typename TFunc>
struct currier;

template<typename TRet, typename TArg>
struct currier< std::function<TRet(TArg)> >
{
	using type = std::function<TRet(TArg)>;
	const type result;

	currier(const type fun) : result(fun) {}
};

template<typename TRet, typename TArgHead, typename ...TArgs>
struct currier< std::function<TRet(TArgHead, TArgs...)> >
{
	using remaining_type = typename currier< std::function<TRet(TArgs...)> >::type;
	using type = std::function<remaining_type(TArgHead)>;

	const type result;

	currier(const std::function<TRet(TArgHead, TArgs...)> fun) : result(
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
		fn(TypeBinder<TArgHead>::from_mrb_value(mrb, args[idx])),
		args);
}


template <int idx, typename TArgHead>
static void void_func_caller(
	mrb_state* mrb,
	typename currier< std::function< void(TArgHead) > >::type fn,
	mrb_value* args)
{
	fn(TypeBinder<TArgHead>::from_mrb_value(mrb, args[idx]));
}

template <int idx, typename TArgHead, typename TArgHead2, typename ...TArgs>
static void void_func_caller(
	mrb_state* mrb,
	typename currier< std::function< void(TArgHead, TArgHead2, TArgs...) > >::type fn,
	mrb_value* args)
{
	void_func_caller<idx + 1, TArgHead2, TArgs...>(
		mrb,
		fn(TypeBinder<TArgHead>::from_mrb_value(mrb, args[idx])),
		args);
}

template<typename TRet, typename ... TArgs>
struct mruby_func_called_returner
{
	static mrb_value call(mrb_state* mrb, std::function<TRet(TArgs...)> func, mrb_value* args)
	{
		auto curried = curry(func);
		TRet result = func_caller<0, TRet, TArgs...>(mrb, curried, args);
		return TypeBinder<TRet>::to_mrb_value(mrb, result);
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
		return TypeBinder<TRet>::to_mrb_value(mrb, result);
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


#endif // __MRUBYFUNCTIONAL_HPP__
