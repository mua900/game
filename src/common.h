#ifndef _COMMON_H
#define _COMMON_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <array>

#define IS_MAX_UNSIGNED(x) ((x)+1==0)
#define BIT(x) (1U << (x))

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


unsigned int pop_count(u64 x);

#ifdef _MSC_VER

#include <intrin.h>

#define NORETURN __declspec(noreturn)

static inline unsigned int msvc_trailing_zeros(u64 x)
{
    unsigned long pos = 0;
    unsigned char is_zero = _BitScanForward64(&pos, x);
    // @note no checking for zero here since we assume non-zero input.
    return pos;
}

static inline unsigned int msvc_leading_zeros(u64 x)
{
    unsigned long pos = 0;
    unsigned char is_zero = _BitScanReverse64(&pos, x);
    // @note no checking for zero here since we assume non-zero input.
    return 63 - pos;
}

#define POP_COUNT(x)      pop_count(x)
#define LEADING_ZEROS(x)  msvc_leading_zeros(x)
#define TRAILING_ZEROS(x) msvc_trailing_zeros(x)

#else // _MSC_VER

#define NORETURN __attribute__((noreturn))

#define POP_COUNT(x)      __builtin_popcountll(x)
#define LEADING_ZEROS(x)  __builtin_clzll(x)
#define TRAILING_ZEROS(x) __builtin_ctzll(x)

#endif

#define ASSERT(x)   do {    \
        if (!(x)) {             \
            fprintf(stderr, "-----*****----- Assertion failed at %s:%d   %s\n", __FILE__, __LINE__, #x); \
            exit(1);    \
        }               \
    } while(0)


NORETURN
void panic(char const* const msg);

#define NOT_IMPLEMENTED(x) panic(x " not implemeneted");

int pop_lsb(u64* x);
int pop_msb(u64* x);

int lsb_index(u64 x);
int msb_index(u64 x);

struct Find_Result {
	int index = 0;
	bool found = false;

    Find_Result(int index, bool found) : index(index), found(found) {}
    Find_Result() {}
};

struct BinaryData {
	u8* data = nullptr;
	size_t size = 0;

    BinaryData()
    {}

    BinaryData(size_t p_size)
    {
        data = (u8*) malloc(p_size);
        if (!data) panic("Memory allocation failure");
        size = p_size;
    }

    ~BinaryData() {
        if (data) {
            free(data);

            data = nullptr;
            size = 0;
        }
    }
};

int string_length(const char* cstr);

struct String {
    const char* data = NULL;
    int size = 0;

    String () {}
	explicit String (const char* d) : data(d), size(string_length(d)) {}
    String (const char* d, int s) : data(d), size(s) {}
	String (const BinaryData& b) : data((const char*)b.data), size(b.size) {}

    bool operator==(const String& other) const;
    void print(bool newline = false) const;
	void trim();
};

#define STRING_EMPTY ((String){.data=NULL,.size=0})
#define CSTRING_LENGTH(s) (sizeof(s)-1)
#define MAKE_STRING(s) (String){.data=s,.size=CSTRING_LENGTH(s)}  // not used

#define SCOPE_STRING_EXP(p_s, p_name, p_size)				\
	char p_name[p_size];  \
	memcpy(p_name, p_s.data, p_s.size);			\
	p_name[p_s.size] = '\0';

#define SCOPE_STRING(str, name) SCOPE_STRING_EXP(str, name, 256)

String make_string(const char* s);
bool string_compare(String s1, String s2);
bool string_starts_with(String s, String start);
int string_match_start(String s1, String s2);
int string_match_character(const String s, int offset, char c);
String string_slice(String s, int start, int end);
String string_slice_to_character(String s, int start, char c);
std::array<String, 2> string_cut_from_character(String s, char c);
String string_get_extension(String s);
String string_copy(String s);

int string_to_integer(String s, bool* success);
double string_to_real(String s, bool* success);

struct String_Builder {
    char* buffer = NULL;
    int buffer_capacity = 0;
    int size = 0;

    String_Builder() {
        create(128);
    }

    String_Builder(int initial_capacity);

    ~String_Builder() {
        if (buffer) {
            free(buffer);
        }
    }

    void create(int initial_capacity);
    void append(String string);
    void append_char(char ch);
    void append_integer(int n);
    void append_hex(int n);
	void append_float(float n);
    const char* c_string();
    void remove(int amount);  // remove the last n characters from the buffer
    void remove_slice(int start, int end);
    void clear_and_append(String s);
    void free_buffer();
    void clear();
    String to_string();
    String slice(int start, int end);
    int ensure_size(int size);
private:
    void resize();
};

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define MIN(x,y) (((x) > (y)) ? (y) : (x))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define CLAMP(x, lower, upper) (MIN(upper, MAX(x, lower)))

long get_file_size(FILE* file);

struct File {
	FILE* handle = nullptr;

    File() {}
    File(FILE* handle) : handle(handle) {}

	bool open(const char* filepath, const char* access) {
        if (handle) fclose(handle);
        handle = fopen(filepath, access);
        return handle ? true : false;
	}

	bool open(String filepath, const char* access) {
        if (handle) fclose(handle);

		SCOPE_STRING(filepath, buffer);

		handle = fopen(buffer, access);
		return handle ? true : false;
	}

	~File() {
        if (handle)
        {
            fclose(handle);
        }
	}

	void write_string(String s);
	void write_number(double n);
	void write_integer(u64 n);
	void write_byte(u8 byte);
	void write_data(BinaryData data);

	String read_string() const;
	double read_number() const;
	u64 read_integer() const;
	int read_byte() const;
	BinaryData read_data() const;
};

struct FileText {
	FILE* handle = nullptr;

    FileText() {}
    FileText(FILE* handle) : handle(handle) {}

	bool open(const char* filepath, const char* access) {
        if (handle) fclose(handle);

		handle = fopen(filepath, access);
		return handle ? true : false;
	}

	bool open(String filepath, const char* access) {
        if (handle) fclose(handle);

		SCOPE_STRING(filepath, buffer);

		handle = fopen(buffer, access);
		return handle ? true : false;
	}

	~FileText() {
        if (handle)
        {
            fclose(handle);
        }
	}

	void write_string(String s, bool nline = false);
	void write_number(double n, bool nline = false);
	void write_integer(u64 n, bool nline = false);
	void write_character(char c);
};

bool load_file(const char* filepath, BinaryData& bdata);
bool load_file_text(const char* filepath, String_Builder& builder);

static inline bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static inline bool is_alpha_lower(char c)
{
    return c >= 'a' && c <= 'z';
}

static inline bool is_alpha_upper(char c)
{
    return c >= 'A' && c <= 'Z';
}

static inline bool is_alpha(char c)
{
    return is_alpha_lower(c) || is_alpha_upper(c);
}

static inline bool is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

static inline char to_lower_ascii(char c)
{
    return is_alpha_upper(c) ? (c - 'A' + 'a') : (c);
}

#define BOOL_STRING(b) ((b) ? ("true") : ("false"))

#endif // _COMMON_H