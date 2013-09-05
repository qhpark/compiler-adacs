/* -----------------------------------------------------
 * DFA state generator -enum token types, DFA lookup table
 *                   Terminal state....
 *
 * Jan 12 2006 -- team@wcri
 * -----------------------------------------------------
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "debug.h"
#include "list.h"

/* list of states */

/* default I/O stream */
static FILE *out;
static FILE *in;

/* handler type */
typedef int (*func_t) (char *);

/* action handler */
struct action_handler {
	char *action;
	func_t h;
	int done;
};


/* handlers ---------------------------*/


/* RESERVED Action */
/* list of tokens */
struct rsvd_st {
	char *token;
	char *rsvd;	
	struct list_head list;
};
static LIST_HEAD(rsvd_list);

static int RSV_handler (char *data)
{
	struct rsvd_st *st;
	char tok[1024], rsvd[1024];
	DPRINTF("RSV_handler invoked! processing --> %s", data);
	if ( 2 != sscanf(data, "%s %s", tok, rsvd))
		return -1;
	st = (struct rsvd_st *) malloc(sizeof(struct rsvd_st));
	assert(st != NULL);
	st->token = strdup(tok);
	st->rsvd = strdup(rsvd);
	INIT_LIST_HEAD(&st->list);
	list_add_tail(&st->list, &rsvd_list);

	return 0;
}

/* DFA Action */

/* defines, data structures */


#define MAX_STATE 256

struct dfa_st {
	char *state_name;
	int terminal;
	char *token; /* available iff terminal */
	unsigned char trans[256];
};
/* state 0 is reserved */
static struct dfa_st dfa_map[MAX_STATE];
static int max_state = 0;

static int gen_table(char *str)
{
	int i;
	for (i = 0 ; i <= max_state; i++) {
		/* match */
		if (!strcmp(dfa_map[i].state_name, str)) {
			break;
		} 
		if (max_state == i) {
			/* new entry */
			max_state = ++i;	
			dfa_map[max_state].state_name = strdup(str);
			dfa_map[max_state].terminal = 0; 
			dfa_map[max_state].token = NULL; 
			memset(dfa_map[0].trans,0,256); /* sizeof .trans */
			DPRINTF("!! State (%s) is assigned %d\n", str, max_state);
			break;
		}
	}
	
	assert(i < 256);
	return i;
}

static int parse_input(unsigned char *tmp, unsigned char *dst, int next) {
	unsigned char last_success = 0;
	unsigned char c;
	unsigned char upto;
	while (*tmp) {
		switch (*tmp) {
			/* handle escape */
			case '\\':
				tmp++; /* advance */

				/* getchar */
				/* \\ or \- */
				if (*tmp == '\\' || *tmp == '-') {
					c = *tmp;
				/* numeric /xxx/ form */
				} else if (isdigit(*tmp)) {
					c = 0;
					while (*tmp != '\\') { 
						c = c * 10 + (*tmp - '0');
						tmp++;
					}
					if (c == 0) {
						fprintf(stderr,"\\0\\ is not allowed\n");
						return -1;
					}
					DPRINTF("RANGE.ESCAPE: %u\n", c);
				} else { /* all the rest */
					fprintf(stderr, "Error -> \\%c\n", *tmp);
					return -1;
				}

				if (last_success) {
					upto = c;
					for (c = last_success; c <= upto ; c++) {
						dst[c] = next; 
						if (c == 255) break;	
					}
					last_success = 0;

				} else if (tmp[1] == '-') { /* peek next */
					last_success = c; /* advance */
					tmp++;
				} else {
					/* default */
					dst[c] = next; 
				}
				break;

			case '-':
				fprintf(stderr, "you can't have \'-\' here\n");
				return -1;	
				break;
			default:
				if (last_success) {
					for (c = last_success; c <= *tmp ; c++) {
						dst[c] = next; 
						if (c == 255) break;	
					}
					last_success = 0;

				} else if (tmp[1] == '-') { /* peek next */
					last_success = *tmp++; /* advance */
				} else {
					/* default */
					dst[*tmp] = next; 
				}
		}
		tmp++;
	}
	DPRINTF("Parsing input: PASS\n");
#ifdef DEBUG
	{
		int i;
		for (i = 0; i < 256; i++) {
			if ((i % 10) == 0) 
				printf("\n");
			printf("%c(%03d):%-3d  ", isprint(i)?i:' ',i, (unsigned char) dst[i]);
		}
		printf("\n");
	}
#endif
	return 0;
}

static int DFA_handler (char *data)
{
	int ret,terminal ;
	int s_idx, n_idx, len;
	char cstate[60], input[1024], nstate[50], tok[1024];
	unsigned char *set_tmp;
	//char input_map[256];
	struct dfa_st *st;

	DPRINTF("DFA_handler invoked! processing --> %s", data);
	if ( (ret = sscanf(data, "%[^.].%d %s %s %s", cstate, &terminal, input, nstate, tok)) < 4) { 
		return -1;
	}
	if (terminal && ret != 5) {
		fprintf(stderr, "requires a token for the terminal state (%s)\n", cstate);	
		return -1;
	}
	if (!strcmp(cstate,"0")) {
		fprintf(stderr, "Current state name can't be 0. RESERVED\n");
		return -1;
	}
	/* check input format */
	len = strlen(input);
	if ( input[0] != '[' || input[len - 1] != ']') {
		fprintf(stderr, "wrong input format (%s)\n", input);	
		return -1;
	} else {
		input[--len] = '\0';
		set_tmp = (unsigned char*) &input[1];
		DPRINTF("input -> %s\n", set_tmp);
	}


	s_idx = gen_table(cstate);
	n_idx = gen_table(nstate);

	if (parse_input(set_tmp, dfa_map[s_idx].trans, n_idx)){ 
		fprintf(stderr, "wrong input format (%s)\n", set_tmp);	
		return -1;
	}

	if (terminal) {
		dfa_map[s_idx].terminal = 1;
		if (!dfa_map[s_idx].token) 
			dfa_map[s_idx].token = strdup(tok);
	}

	return 0;
}

struct action_handler ah[] = {
	{"RESERVED", RSV_handler, 0},
	{"DFA", DFA_handler, 0},
};

#define NUM_CMD (sizeof(ah) / sizeof(struct action_handler))


/* Code generator --------------------------------------------*/
#define TOP_MESSAGE \
	"// do not change, this file is automatically generated.\n" \
	"// NOTE: this file should be included by only one file\n"  \
	"//       thus, no #ifndef, #endif.\n" \
	"\n\n"
		
#define BOTTOM_MESSAGE \
	"\n\n\n// end of file---------- generated : %s"
/* RESERVED */
static const char* const rsvd_dcl_begin = 
	TOP_MESSAGE
	"// token <-> enum list.\n"
	"// we need to do a linear search.\n"
	"struct __rsvd_st {\n"
	"\tint token;    // defined somewhere else\n"
	"\tchar *rsvd;\n"
	"};\n\n"
	"static struct __rsvd_st rsvd_table[] = {\n";

static const char* const rsvd_dcl_end = 
	"};\n\n" 
	"static const int num_rsvd_tbl = %d;\n";

static const char* const rsvd_tmpl = "\t{ %s, \"%s\" },\n" ;

static void gen_rsvd_table(void)
{
	struct list_head *entry, *tmp;
	struct rsvd_st *st;
	int i = 0;

	fprintf(out, rsvd_dcl_begin); 

	list_for_each_safe(entry, tmp, &rsvd_list) {
		st = list_entry(entry, struct rsvd_st, list); 
		fprintf(out, rsvd_tmpl, st->token, st->rsvd);	
		list_del(entry);
		free(st->token);
		free(st->rsvd);
		free(st);
		i++;
	}
	fprintf(out, rsvd_dcl_end, i); 
}


/* DFA */
static const char* const dfa_dcl_begin = 
	"struct __dfa_st {\n"
	"\tint terminal;  // 1 if terminal, 0 otherwise\n"
	"\tint token;  // defined somewhere else..\n"
	"\tunsigned char map[256]; // points to next state\n"
	"};\n\n"
	"static struct __dfa_st DFA[] = {\n";

static const char* const dfa_dcl_end = 
	"};\n\n" 
	"static const int num_dfa_tbl = %d;\n"
	BOTTOM_MESSAGE;

static const char* const dfa_tmpl_b =
	"\n\t{ /* terminal */ %d, /* token */ %s, {";
	
	
static const char* const dfa_tmpl_e =
	"}\n\t  /* state %d (%s) */\n\t},\n" ;

/* State Table */
static void gen_dfa_table(void)
{
	int i,j;
	time_t stamp = time(NULL);
	fprintf(out, dfa_dcl_begin); 
	/* at least once. 0 is reserved */
	for (i = 0; i <= max_state; i++) {
		assert(!dfa_map[i].terminal || 
		      (dfa_map[i].terminal && dfa_map[i].token));
		
		fprintf(out, dfa_tmpl_b, dfa_map[i].terminal, 
				dfa_map[i].token ? dfa_map[i].token : "0");	
		for (j = 0 ; j < 256 ; j++) {
			if ((j % 20) == 0) 
				printf("\n\t  ");
			fprintf(out, "%2d,", (unsigned) dfa_map[i].trans[j]);	
		}
		fprintf(out, dfa_tmpl_e, i, dfa_map[i].state_name); 
	}
	fprintf(out, dfa_dcl_end, i,ctime(&stamp)); 
}


/* Helper functions */
// uppers
// lowers


int main(int argc, char **argv)
{
	char line_buf[1024];
	char *line;
	char buf[256];
	char *tmp;
	int in_action = -1;
	int i;
	int line_num = 0;

	/* create reserved state 0 entry */
	dfa_map[0].state_name = "0";
	dfa_map[0].terminal = 0;
	dfa_map[0].token = NULL;
	memset(dfa_map[0].trans,0,256); /* sizeof .trans */

	/* get input/output type,
	 * by default, it's standard I/O
	 */
	in = stdin;
	out = stdout;

	while (fgets(line_buf, sizeof(line_buf), in)) {
		/* increment line_num */
		line_num++;

		/* get rid of whitespace */
		line = line_buf;
		while (*line == ' ' || *line == '\t')
			line++;
				
		/* ignore comments */
		if (*line == '#' || *line == '\n') continue;

		/* check for action descriptor */
		if (1 == sscanf( line, "[%s", buf) && (tmp = strchr(buf, ']'))) {
			*tmp = '\0';	
			/* old action is now done */
			if (in_action != -1)
				ah[in_action].done = 1;

			for (i = 0; i < NUM_CMD; i++) {
				if (!strcmp(ah[i].action, buf)) {
					/* Match! */
					if (ah[i].done) {
						fprintf(stderr, "[%s] redefined, quit. line:%d\n",
								buf,line_num);
						exit(1);
					}
					in_action = i;	
					break;
				}
			}
			continue;
		}
	
		/* must be in some action */
		if (in_action == -1) {
			fprintf(stderr,"UNEXPECTED LINE :%s, line: %d\n", line_num);
			exit(2);
		}
			
		/* handle action */
		if (ah[in_action].h(line)) {
			fprintf(stderr,"WARNING: there's an error in [%s], line: %d\n",
					ah[i].action, line_num);
			return 2;
		}
			
		
	}
	/* generate reserved table */
	gen_rsvd_table();
	/* generate dfa table */
	gen_dfa_table();
	return 0;
}
