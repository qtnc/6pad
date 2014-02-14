#ifndef _PCRE_FUNCTIONS_QC3
#define _PCRE_FUNCTIONS_QC3
#include<pcre.h>

#define PCRE_DEFAULT_OPTIONS PCRE_DOTALL | PCRE_MULTILINE | PCRE_NEWLINE_ANY
#define PCRE_COMPILETIME_MASK (PCRE_CASELESS | PCRE_MULTILINE | PCRE_DOTALL | PCRE_EXTENDED | PCRE_EXTRA | PCRE_ANCHORED | PCRE_DOLLAR_ENDONLY | PCRE_UNGREEDY | PCRE_UTF8 | PCRE_NO_UTF8_CHECK | PCRE_DUPNAMES | PCRE_NEWLINE_CR | PCRE_NEWLINE_LF | PCRE_NEWLINE_CRLF | PCRE_NEWLINE_ANY | PCRE_NO_AUTO_CAPTURE | PCRE_FIRSTLINE)
#define PCRE_COMPILETIME_FORCE PCRE_NEWLINE_ANY
#define PCRE_RUNTIME_MASK (PCRE_NOTEOL | PCRE_NOTBOL | PCRE_NOTEMPTY | PCRE_NEWLINE_CR | PCRE_NEWLINE_LF | PCRE_NEWLINE_CRLF | PCRE_NEWLINE_ANY )
#define PCRE_RUNTIME_FORCE PCRE_NEWLINE_ANY


#define preg_compile(r,o) pcre_compile(r, o, &pregerrormsg, &pregerrorof, NULL)

extern char* pregerrormsg;
extern int pregerrorof;

typedef char*(*preg_callback)(pcre*, const char*, int*, int, void*) ;

int preg_match (pcre* regex, int options, const char* str, int offset, int length, int** match) ;
char* preg_replace_callback (pcre* regex, int options, preg_callback callback, const char* str, int offset, int length, int* nReplaces, void* udata);
char* preg_replace (pcre* regex, int options, const char* repl, const char* str, int offset, int length, int *nReplaces);

#endif
