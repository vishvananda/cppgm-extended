// Larger, deterministic application workloads. Each repetition starts from the
// same prepared data; working-set traffic is part of run().
static unsigned words[65536];
static unsigned scratch[65536];
static unsigned merge_work[65536];
static unsigned distance[2048];
static unsigned queue_nodes[32768];
static unsigned queue_keys[32768];
static unsigned char text[262144];
static unsigned char packed[524288];
static unsigned char unpacked[262144];
static unsigned text_size;

static void prepare_applications(unsigned seed)
{
    for (unsigned i = 0; i < 65536; ++i) {
        seed = seed * 1664525u + 1013904223u;
        words[i] = seed;
    }
    // A structured document with varied keys, whitespace, escapes and signed
    // integers. It is also a compressible byte stream for the codec workload.
    const char prefix[] = "{\"items\":[";
    text_size = 0;
    for (unsigned i = 0; prefix[i]; ++i) text[text_size++] = prefix[i];
    for (unsigned i = 0; i < 4096; ++i) {
        const char record[] = "{\"name\":\"item\\n\",\"value\":";
        if (i) text[text_size++] = ',';
        for (unsigned j = 0; record[j]; ++j) text[text_size++] = record[j];
        unsigned value = words[i] & 65535;
        if (value & 1) text[text_size++] = '-';
        char digits[10];
        unsigned count = 0;
        do { digits[count++] = '0' + value % 10; value /= 10; } while (value);
        while (count) text[text_size++] = digits[--count];
        text[text_size++] = '}';
        if ((i & 7) == 0) text[text_size++] = '\n';
    }
    text[text_size++] = ']';
    text[text_size++] = '}';
}

static __attribute__((noinline)) unsigned merge_sort(unsigned seed)
{
    for (unsigned i = 0; i < 65536; ++i) scratch[i] = words[i] ^ seed;
    unsigned* src = scratch;
    unsigned* dst = merge_work;
    for (unsigned width = 1; width < 65536; width *= 2) {
        for (unsigned first = 0; first < 65536; first += 2 * width) {
            unsigned a = first, b = first + width;
            unsigned ae = b, be = first + 2 * width;
            for (unsigned out = first; out < be; ++out)
                dst[out] = a < ae && (b == be || src[a] <= src[b]) ? src[a++] : src[b++];
        }
        unsigned* swap = src; src = dst; dst = swap;
    }
    unsigned result = 0;
    for (unsigned i = 0; i < 65536; ++i) result += src[i] * (i + 1);
    return result;
}

static void push_queue(unsigned& size, unsigned node, unsigned key)
{
    unsigned at = size++;
    while (at && key < queue_keys[(at - 1) / 2]) {
        queue_nodes[at] = queue_nodes[(at - 1) / 2];
        queue_keys[at] = queue_keys[(at - 1) / 2];
        at = (at - 1) / 2;
    }
    queue_nodes[at] = node;
    queue_keys[at] = key;
}

static __attribute__((noinline)) unsigned shortest_paths(unsigned seed)
{
    for (unsigned i = 0; i < 2048; ++i) distance[i] = 0xffffffffu;
    unsigned source = seed & 2047, size = 0;
    distance[source] = 0;
    push_queue(size, source, 0);
    while (size) {
        unsigned node = queue_nodes[0], key = queue_keys[0];
        unsigned last_node = queue_nodes[--size], last_key = queue_keys[size], at = 0;
        while (2 * at + 1 < size) {
            unsigned child = 2 * at + 1;
            if (child + 1 < size && queue_keys[child + 1] < queue_keys[child]) ++child;
            if (last_key <= queue_keys[child]) break;
            queue_nodes[at] = queue_nodes[child]; queue_keys[at] = queue_keys[child];
            at = child;
        }
        queue_nodes[at] = last_node; queue_keys[at] = last_key;
        if (distance[node] != key) continue;
        for (unsigned edge = 0; edge < 8; ++edge) {
            unsigned x = words[node * 8 + edge];
            unsigned next = edge ? (x >> 12) & 2047 : (node + 1) & 2047;
            unsigned candidate = key + 1 + (x & 255);
            if (candidate < distance[next]) {
                distance[next] = candidate;
                push_queue(size, next, candidate);
            }
        }
    }
    unsigned result = 0;
    for (unsigned i = 0; i < 2048; ++i) result += distance[i] * (i + 1);
    return result;
}

// Recursive-descent JSON parsing, including nesting, escaped strings and
// signed integer conversion. Hash parsed tokens, rather than just bytes.
struct JsonReader {
    unsigned at, result, salt;
    void space() { while (at < text_size && text[at] <= ' ') ++at; }
    void token(unsigned value) { result = result * 33u + (value ^ salt); }
    void string() {
        ++at;
        unsigned hash = 0;
        while (text[at] != '"') {
            unsigned c = text[at++];
            if (c == '\\') { c = text[at++]; if (c == 'n') c = '\n'; }
            hash = hash * 31u + c;
        }
        ++at;
        token(hash);
    }
    void value() {
        space();
        unsigned c = text[at];
        if (c == '"') { string(); return; }
        if (c == '{' || c == '[') {
            unsigned close = c == '{' ? '}' : ']';
            ++at; token(c); space();
            while (text[at] != close) {
                if (c == '{') { string(); space(); ++at; }
                value(); space();
                if (text[at] == ',') { ++at; space(); }
            }
            ++at; token(close);
            return;
        }
        bool negative = c == '-';
        if (negative) ++at;
        unsigned number = 0;
        while (at < text_size && text[at] >= '0' && text[at] <= '9')
            number = number * 10u + text[at++] - '0';
        token(negative ? 0u - number : number);
    }
};

static __attribute__((noinline)) unsigned parse_document(unsigned seed)
{
    JsonReader reader = {0, 0, seed};
    reader.value();
    return reader.at == text_size ? reader.result : 0;
}

// LZSS with a 4096-entry last-match table, 16-bit offsets and up to 130
// bytes per match. Decode every stream and check the complete round trip.
static __attribute__((noinline)) unsigned encode_document()
{
    for (unsigned i = 0; i < 4096; ++i) scratch[i] = 0xffffffffu;
    unsigned at = 0, out = 0;
    while (at < text_size) {
        unsigned length = 0, previous = 0;
        if (at + 2 < text_size) {
            unsigned hash = (text[at] * 251u + text[at + 1] * 11u + text[at + 2]) & 4095;
            previous = scratch[hash]; scratch[hash] = at;
            if (previous < at && at - previous <= 65535)
                while (length < 130 && at + length < text_size &&
                       text[previous + length] == text[at + length]) ++length;
        }
        if (length >= 3) {
            packed[out++] = 128 + length - 3;
            unsigned offset = at - previous;
            packed[out++] = offset; packed[out++] = offset >> 8;
            at += length;
        } else {
            packed[out++] = 0; packed[out++] = text[at++];
        }
    }
    return out;
}

static __attribute__((noinline)) unsigned decode_document(unsigned size, unsigned seed)
{
    unsigned at = 0, out = 0;
    while (at < size) {
        unsigned tag = packed[at++];
        if (tag < 128) unpacked[out++] = packed[at++];
        else {
            unsigned offset = packed[at] + (unsigned(packed[at + 1]) << 8); at += 2;
            for (unsigned count = tag - 128 + 3; count; --count) {
                unpacked[out] = unpacked[out - offset]; ++out;
            }
        }
    }
    unsigned hash = seed;
    if (out != text_size) return 0;
    for (unsigned i = 0; i < out; ++i) {
        if (unpacked[i] != text[i]) return 0;
        hash = hash * 33u + unpacked[i];
    }
    return hash ^ size;
}

static unsigned run_application(unsigned kind, unsigned seed)
{
    if (kind == 4) return merge_sort(seed);
    if (kind == 5) return shortest_paths(seed);
    if (kind == 6) return parse_document(seed);
    return decode_document(encode_document(), seed);
}
