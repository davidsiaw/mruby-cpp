#ifndef __MRUBYNATIVEARRAYTYPEBINDER_HPP__
#define __MRUBYNATIVEARRAYTYPEBINDER_HPP__

template<>
struct TypeBinder<NativeArray>
{
	static mrb_value to_mrb_value(mrb_state* mrb, NativeArray ary)
	{
		if (!ary.instantiated())
		{
			ary.create(mrb);
		}
		return ary.instance();
	}

	static NativeArray from_mrb_value(mrb_state* mrb, mrb_value val)
	{
		if (mrb_array_p(val))
		{
			return NativeArray(std::shared_ptr<mrb_state>(mrb), val);
		}

		throw TypeError("Not an array");
	}
};

#endif // __MRUBYNATIVEARRAYTYPEBINDER_HPP__
