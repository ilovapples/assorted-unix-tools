#include "arg_parse.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitset.h"
#include "cvi/dvec/dvec.h"

struct arg_parser ap_create(int argc, char **argv)
{
	struct arg_parser ap = {
		.args = { argc, argv },
		.dashdash_pos = _none_,
		.dash_pos = _none_,
	};
	ap.args_used = bitset_create_zeroed(argc);

	return ap;
}

void ap_destroy(struct arg_parser *ap)
{
	bitset_destroy(ap->args_used);
	dv_destroy(ap->declared_options);
	dv_destroy(ap->declared_positional_options);
}

static bool ap_argument_is_option(const char *arg)
{
	return arg[0] == '-';
}

// declare a long-form option in the `arg_parser` ap
//
// only supports '--field value' or '--bool-field' syntax
//
// if 'type_str' is "bool", returns "" if option is passed, NULL if not
// otherwise, returns the next argument in the list if possible, NULL if not possible,
// or the option wasn't passed in the first place.
const char *ap_long_option(
		struct arg_parser *ap,
		const char *name,
		const char *desc,
		const char *type_str)
{
	// copy made so we aren't doing a millionpointer dereferences of ap->specified_options
	__auto_type declared_options_copy = ap->declared_options;
	const bool type_is_bool = strcmp(type_str, "bool") == 0;
	dv_push(declared_options_copy, ((struct ap_option) {
			.short_name = '\0',
			.decl = {
				// make it a bit faster when rereading type_str from other functions
				.type_str = (type_is_bool) ? NULL : type_str,
				.description = desc,
				.name = name,
			},
	}));
	ap->declared_options = declared_options_copy;

	ap->err = ap_err__option_not_present;
	const ap_argv_t args = ap->args;
	for (size_t i = 1; i < (size_t)args.argc; ++i)
	{
		if (bitset_is_set_at(ap->args_used, i)) continue;
		const char *cur_arg = args.argv[i];
		if (cur_arg[0] == '-' && cur_arg[1] == '-') {
			if (strcmp(cur_arg+2, name) == 0) {
				if (type_is_bool) {
					bitset_set_at(ap->args_used, i);
					return ""; // empty string, but detectably non-null
				}
				if (i < (size_t)args.argc - 1) {
					bitset_set_at(ap->args_used, i);
					bitset_set_at(ap->args_used, i+1);
					return args.argv[i+1];
				} else {
					ap->err = ap_err__nonbool_option_is_empty;
					return NULL;
				}
			} else if (cur_arg[2] == '\0' && ap->dashdash_pos.is_null == true) {
				ap->dashdash_pos = some(opt_usize, i);
				bitset_set_at(ap->args_used, i);
			}
		}
	}

	return NULL;
}

// declare a short-form option
//
// only supports -abcde where -a, -b, -c, -d, -e are short form flags (and can be combined in any way)
// short-form doesn't actually have to be alphabetic, but can't be '+'.
//
// NULL means an error, unless ap.err is ap_err__option_not_present, in which case it is a non-error 'false'.
// if not NULL, the option was present. A Boolean option should test the return value against NULL.
const char *ap_short_option(
		struct arg_parser *ap,
		char letter,
		const char *associated_long_name,
		const char *desc,
		const char *type_str)
{
	const bool type_is_bool = strcmp(type_str, "bool") == 0;
	// associated_long_name and desc null-ness is mutually exclusive,
	// but description is checked first, so it won't necessarily be an issue
	__auto_type declared_options_copy = ap->declared_options;
	if (desc == NULL) {
		struct ap_option *item;
		dv_foreach(declared_options_copy, item)
		{
			if (item->decl.name && strcmp(item->decl.name, associated_long_name) == 0) {
				item->short_name = letter;
				break;
			}
		}
	} else if (associated_long_name == NULL) {
		dv_push(declared_options_copy, ((struct ap_option) {
				.short_name = letter,
				.decl = {
					.type_str = (type_is_bool) ? NULL : type_str,
					.description = desc,
					.name = NULL,
				}
		}));
	}
	ap->declared_options = declared_options_copy;

	ap->err = ap_err__option_not_present;

	const ap_argv_t args = ap->args;
	for (size_t i = 1; i < args.argc; ++i)
	{
		if (bitset_is_set_at(ap->args_used, i)) continue;
		char *cur_arg = args.argv[i];
		if (cur_arg[0] == '-' && cur_arg[1] != '-') {
			if (cur_arg[1] == '\0' && ap->dash_pos.is_null == true) { // '-'
				ap->dash_pos = some(opt_usize, i);
				bitset_set_at(ap->args_used, i);
				continue;
			}
			bool had_unused_chars = false;
			for (char *cur = cur_arg+1; *cur; ++cur)
			{
				// genius logic: replace used short args with '+', mark the whole argument used when all are '+'
				if (*cur == '+') continue;
				had_unused_chars = true;
				if (*cur == letter) {
					if (type_is_bool) {
						*cur = '+';
						return "";
					}
					if (i < args.argc-1) {
						*cur = '+';
						bitset_set_at(ap->args_used, i+1);
						return args.argv[i+1];
					} else {
						ap->err = ap_err__nonbool_option_is_empty;
						return NULL;
					}
				}
			}
			if (!had_unused_chars) bitset_set_at(ap->args_used, i);
		}
	}

	return NULL;
}

const char *ap_long_short_option(
		struct arg_parser *ap,
		const char *long_name,
		char short_name,
		const char *desc,
		const char *type_str)
{
	const char *long_option = ap_long_option(ap, long_name, desc, type_str);
	const char *short_option = ap_short_option(ap, short_name, long_name, NULL, type_str);

	return (long_option ? long_option : short_option);
}

// found in order of declaration, so positional args should
// be declared in order of appearance
const char *ap_positional_option(
		struct arg_parser *ap,
		const char *name,
		const char *desc,
		const char *type_str) {
	__auto_type decld_pos_options_copy = ap->declared_positional_options;
	const bool type_is_bool = strcmp(type_str, "bool") == 0;
	dv_push(decld_pos_options_copy, ((struct ap_option_decl) {
		.type_str = (type_is_bool) ? NULL : type_str,
		.description = desc,
		.name = name,
	}));
	ap->declared_positional_options = decld_pos_options_copy;

	const size_t cur_pos_idx = dv_n(decld_pos_options_copy);

	ap->err = ap_err__option_not_present;
	const ap_argv_t args = ap->args;
	for (size_t i = 1, pos_arg_n = 0; i < (size_t)args.argc; ++i) {
		if (ap_argument_is_option(args.argv[i])) continue;
		++pos_arg_n;
		if (bitset_is_set_at(ap->args_used, i)) continue;

		if (pos_arg_n == cur_pos_idx) {
			bitset_set_at(ap->args_used, i);
			return args.argv[i];
		}
	}

	return NULL;
}


size_t ap_options_column_2_offset = 30;
void ap_print_declared_options(struct arg_parser ap, FILE *crestrict fp)
{
	if (dv_n(ap.declared_positional_options) > 0) {
		fputs("positional options:\n"
		      "  ", fp);
		const struct ap_option_decl *it;
		dv_foreach(ap.declared_positional_options, it)
			fprintf(fp, "[%s%s%s] ",
				(it->name) ? it->name : "",
				(it->name) ? ": " : "",
				(it->type_str) ? it->type_str : "bool");
		fputs("\npositional option descriptions:\n", fp);
		dv_foreach(ap.declared_positional_options, it)
		{
			fprintf(fp, "  [%s%s%s]\x1b[%zuG%s\n",
				(it->name) ? it->name : "",
				(it->name) ? ": " : "",
				(it->type_str) ? it->type_str : "bool",
				ap_options_column_2_offset,
				it->description);
		}

		fputc('\n', fp);
	}

	if (dv_n(ap.declared_options) > 0) {
		fputs("options:\n", fp);
		const struct ap_option *it;
		dv_foreach(ap.declared_options, it)
		{
			fputs("  ", fp);
			if (it->decl.name != NULL) fprintf(fp, "--%s", it->decl.name);
			if (it->short_name != '\0') {
				if (it->decl.name == NULL)
					fprintf(fp, "-%c", it->short_name);
				else
					fprintf(fp, ", -%c", it->short_name);
			}
			if (it->decl.type_str) fprintf(fp, " [%s]", it->decl.type_str);
			fprintf(fp, "\x1b[%zuG%s\n", ap_options_column_2_offset, it->decl.description);
		}
	}
}
