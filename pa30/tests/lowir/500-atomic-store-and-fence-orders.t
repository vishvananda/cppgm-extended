// An atomic store or thread fence whose order is not a constant branches
// on it: below seq_cst's 5 the store is release and the fence acq_rel,
// otherwise both are seq_cst. A constant order, and the order of any other
// operation that is not a constant, take no branch.
void store_n(int* p, int v, int order)
{
	__atomic_store_n(p, v, order);
}

void store_generic(long* p, long* v, int order)
{
	__atomic_store(p, v, order);
}

void clear(bool* p, int order)
{
	__atomic_clear(p, order);
}

void fences(int order)
{
	__atomic_thread_fence(order);
	__atomic_signal_fence(order);
}

int others(int* p, int v, int order)
{
	__atomic_store_n(p, v, __ATOMIC_RELEASE);
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
	int seen = __atomic_load_n(p, order);
	return seen + __atomic_exchange_n(p, v, order);
}
