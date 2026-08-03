#pragma once

template<class... Types>
int pack_sum(Types&&... values);

extern "C" int host_pack_sum();
