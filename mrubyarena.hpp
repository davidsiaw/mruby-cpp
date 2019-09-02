#ifndef __MRUBYARENA_HPP__
#define __MRUBYARENA_HPP__

/*
 * This class represents a scope where, when objects are made natively
 * using functions such as `mrb_data_object_alloc` or `mrb_str_new`
 * they should be owned by the mruby gc. You use this class like this:
 *
 *     Arena arena(mrb);
 *     {
 *       // do allocations here
 *     }
 *
 * The braces do nothing important. They are just for readability.
 *
 * This construct is very important because not doing this causes
 * what would start looking like memory leaks since all allocations
 * made in native code are tracked on a stack called the gc arena,
 * and there is no way to deref only specific objects from it.
 *
 * Read more about this in the mruby docs folder
 */
class Arena
{
	mrb_state* mrb;
	int arena_idx;
	
public:
	Arena(mrb_state* mrb) : mrb(mrb)
	{
		arena_idx = mrb_gc_arena_save(mrb);
	}

	~Arena()
	{
		mrb_gc_arena_restore(mrb, arena_idx);
	}

	void protect(mrb_value obj)
	{
		mrb_gc_protect(mrb, obj);
	}
};

#endif // __MRUBYARENA_HPP__
