typedef long long Wide __attribute__((vector_size(16)));
typedef long long Narrow __attribute__((vector_size(8)));
Wide value{Narrow{1}};
