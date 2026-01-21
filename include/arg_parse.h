#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#include <stdio.h>
#include <stddef.h>

#include "bitset.h"
#include "cvi/dvec/dvec.h"

#ifdef __cplusplus
#define crestrict
#else
#define crestrict restrict
#endif

enum ap_error {
	nil,
	// not an error
	ap_err__option_not_present,
	ap_err__nonbool_option_is_empty,
};

typedef struct ap_argv_t {
	int argc;
	char **argv;
} ap_argv_t;

struct ap_option_decl {
	const char *description, *type_str, *name;
};

struct ap_option {
	char short_name;
	struct ap_option_decl decl;
};

#define optional(_T) struct { bool is_null; _T val; }
typedef optional(size_t) opt_usize;
#define _none_ { .is_null = true }
#define none(_oT) ((_oT) _none_)
#define _some_(_v) { .is_null = false, .val = (_v) }
#define some(_oT, _v) ((_oT) _some_(_v))

struct arg_parser {
	ap_argv_t args;
	bitset args_used;

	dvec_t(struct ap_option) declared_options;
	dvec_t(struct ap_option_decl) declared_positional_options;

	opt_usize dashdash_pos;
	opt_usize dash_pos;

	// only set/changed when a function returns an error value (like NULL);
	// unsafe to access after a function returns non-NULL (may have a nonsensical value)
	enum ap_error err;
};

struct arg_parser ap_create(int argc, char **argv);
void ap_destroy(struct arg_parser *ap);
const char *ap_long_option(
		struct arg_parser *ap,
		const char *name,
		const char *desc,
		const char *type_str);
const char *ap_short_option(
		struct arg_parser *ap,
		char letter,
		const char *associated_long_name,
		const char *desc,
		const char *type_str);

const char *ap_short_option(
		struct arg_parser *ap,
		char letter,
		const char *associated_long_name,
		const char *desc,
		const char *type_str);

const char *ap_long_short_option(
		struct arg_parser *ap,
		const char *long_name,
		char short_name,
		const char *desc,
		const char *type_str);

const char *ap_positional_option(
		struct arg_parser *ap,
		const char *name,
		const char *desc,
		const char *type_str);

void ap_print_declared_options(struct arg_parser ap, FILE *crestrict fp);


#endif /* ARG_PARSE_H */
