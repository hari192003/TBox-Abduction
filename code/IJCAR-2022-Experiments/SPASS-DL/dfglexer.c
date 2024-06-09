/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *        DFG LEXER                                       * */
/* *                                                        * */
/* *  $Module: DFG_LEXER_H                                  * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001      * */
/* *  MPI fuer Informatik                                   * */
/* *                                                        * */
/* *  This program is free software; you can redistribute   * */
/* *  it and/or modify it under the terms of the FreeBSD    * */
/* *  Licence.                                              * */
/* *                                                        * */
/* *  This program is distributed in the hope that it will  * */
/* *  be useful, but WITHOUT ANY WARRANTY; without even     * */
/* *  the implied warranty of MERCHANTABILITY or FITNESS    * */
/* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * */ 
/* *  for more details.                                     * */
/* *                                                        * */
/* *                                                        * */
/* $Revision: 1.0 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016/03/23 16:53:37 $                             * */
/* $Author: weidenb $                                       * */
/* *                                                        * */
/* *             Contact:                                   * */
/* *             Christoph Weidenbach                       * */
/* *             MPI fuer Informatik                        * */
/* *             Stuhlsatzenhausweg 85                      * */
/* *             66123 Saarbruecken                         * */
/* *             Email: spass@mpi-inf.mpg.de                * */
/* *             Germany                                    * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/


#include <ctype.h>

#include "dfg_diagnostic.h"
#include "dfglexer.h"
#include "dfg_string_table.h"
#include "dfg_util.h"

token_t token;

/**
 * Internal state of the lexer.
 */
static struct {
	FILE*       f;        /**< The input stream. */
	char*       buf;
	char const* cur;
	char const* end;
	int         c;        /**< The current input character. */
	pos_t       pos;      /**< The position in the input. */
	char*       text;     /**< Text buffer for lexing words. */
	size_t      capacity; /**< Capacity of the text buffer. */
	size_t      size;     /**< Size of the word in the text buffer. */
} input;

/**
 * Append the char c to the text buffer.
 */
static inline void text_append(char const c)
{
	/* Resize the text buffer, if it is full. */
	if (input.size == input.capacity) {
		input.text = realloc(input.text, input.capacity *= 2);
	}
	input.text[input.size++] = c;
}

#define BUF_SIZE (4 * 1024)

/**
 * Get the next character from the input.
 *
 * Includes the current column by one.
 */
static inline int input_get(void)
{
	input.pos.col += 1;
	if (input.cur == input.end) {
		size_t const res = fread(input.buf, 1, BUF_SIZE, input.f);
		input.cur = input.buf;
		input.end = input.buf + res;
		if (res == 0) {
			if (ferror(input.f)) {
			  misc_StartUserErrorReport();
			  dfg_UserErrorReportF(&input.pos, "Error in File");
			  misc_FinishUserErrorReport();
			}
			return EOF;
		}
	}
	return *input.cur++;
}

void lexer_set_input(FILE* const f, char const* const name)
{
	input.f        = f;
	if (!input.buf)
		input.buf = MALLOCNZ(char, BUF_SIZE);
	input.pos.name = name;
	input.pos.line = 1;
	input.pos.col  = 0;
	input.c        = input_get();
}

enum {
	CHAR_NONE    = 0,
	CHAR_DIGIT   = 1U << 0,
	CHAR_LETTER  = 1U << 1,
	CHAR_SPECIAL = 1U << 2,
	CHAR_RESVAR  = 1U << 3
};

static unsigned char char_class[256];

/**
 * Checks whether the character c is a digit.
 */
static inline bool is_digit(int const c)
{
	return '0' <= c && c <= '9';
}

/**
 * Checks whether the character c may start an identifier.
 */
static inline bool is_ident(int const c)
{
	return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_';
}

/**
 * Set the kind and text of the current token.
 */
static inline void set_token(token_kind_t const kind, char const* const text)
{
	token.kind = kind;
	token.text = text;
}

void lexer_lextext(void)
{
  char c;
  text_append('{');
  text_append('*');
  c = input.c;
  for (;;) {
    if (c == '*') {
      text_append(c);
      c = input_get();
      if (c == '}') {
	text_append(c);
	input.c = input_get();
	text_append('\0');
	set_token(T_TEXT, input.text);
	input.size = 0;
	return;
      } else if (c == EOF) {
	goto unterminated_comment;
      }
    } else if (c == EOF) {
    unterminated_comment:
      misc_StartUserErrorReport();
      dfg_UserErrorReportF(&token.pos, "unterminated text");
      misc_FinishUserErrorReport();
      break;
    }
    if(c == '\n'){
      input.pos.line += 1;
      input.pos.col   = 0;
    }
    text_append(c);
    c = input_get();
  }
}

void lexer_next(void)
{
	int c = input.c;
	for (;;) {
		token.pos = input.pos;
		switch (c) {
			char invalid;

		case EOF:
			set_token(T_EOF, "EOF");
			return;

		case '\n':
			input.pos.line += 1;
			input.pos.col   = 0;
			/* FALLTHROUGH */
		case ' ':
		case '\f':
		case '\r':
		case '\t':
		case '\v':
			/* Skip whitespace. */
			c = input_get();
			continue;

		case '\'':
			for (;;) {
				c = input_get();
				if (c == '\'') {
					input.c = input_get();
					text_append('\0');
					set_token(T_FILENAME, input.text);
					input.size = 0;
					return;
				} else if (c == T_EOF) {
				  misc_StartUserErrorReport();
				  dfg_UserErrorReportF(&token.pos, "unterminated filename");
				  misc_FinishUserErrorReport();
					break;
				}
				text_append(c);
			}
			continue;

		case '{':
			input.c = input_get();
			if (input.c == '*') {
			  set_token(T_LPARSTAR, "{*");
			  input.c = input_get();
			} else {
			  set_token(T_LCBRACE, "{");
			}
			return;
		case '}': set_token(T_RCBRACE,    "}"); input.c = input_get(); return;
		case '*':
			c = input_get();
			if (c == '}') {
			  set_token(T_RPARSTAR, "*}");
			  input.c = input_get();
			  return;
			} else {
				invalid = '*';
				goto invalid_char;
			}
		case '%':
			do {
				c = input_get();
			} while (c != '\n' && c != EOF);
			continue;

		case '-':
			c = input_get();
			if (c == '>') {
				set_token(T_MINUS_GREATER, "->");
				input.c = input_get();
			} else if (is_digit(c)) {
			  text_append('-');
				goto number;
			} else {
			  invalid = 'c';
			  goto invalid_char;
			}
			return;

		case '|':
			c = input_get();
			if (c == '|') {
				set_token(T_PIPE_PIPE, "||");
				input.c = input_get();
				return;
			} else {
				invalid = 'c';
				goto invalid_char;
			}
		case '(': set_token(T_LPAREN,    "("); input.c = input_get(); return;
		case ')': set_token(T_RPAREN,    ")"); input.c = input_get(); return;
		case ',': set_token(T_COMMA,     ","); input.c = input_get(); return;
		case '.': set_token(T_PERIOD,    "."); input.c = input_get(); return;
		case ':': set_token(T_COLON,     ":"); input.c = input_get(); return;
		case '[': set_token(T_LBRACKET,  "["); input.c = input_get(); return;
		case ']': set_token(T_RBRACKET,  "]"); input.c = input_get(); return;

		default: {
			if (is_digit(c)) {
				/* Read a number. */
number:
				do {
					text_append(c);
					c = input_get();
				} while (is_digit(c));
				if (char_class[c] & (CHAR_LETTER | CHAR_SPECIAL) && input.text[0] != '-')
					goto ident;
				/* Append the terminating NUL character to the text buffer. */
				text_append('\0');
				set_token(T_NUMBER, input.text);
			} else if (char_class[c] & (CHAR_LETTER | CHAR_SPECIAL)) {
				/* Read an identifier or keyword or reserved variable. */
ident:
			  text_append(c);
			  c = input_get();
				
				while (c != EOF && char_class[c] & (CHAR_DIGIT | CHAR_LETTER | CHAR_SPECIAL)){
				  text_append(c);
				  c = input_get();
				}
				/* Append the terminating NUL character to the text buffer. */
				text_append('\0');
				/* Find or insert the current string in(to) the string table.
				 * If the string is not in the table yet, it is inserted as T_IDENTIFIER.
				 * If it is contained already, the kind is not changed.
				 * This is used to identify keywords by inserting them into the string table beforehand in lexer_init().
				 */
				string_t const* const s = string_table_insert(input.text, input.size, T_IDENTIFIER);
				set_token(s->kind, s->text);
			} else {
				invalid = c;
invalid_char:
				if (isprint(invalid)) {
				  misc_StartUserErrorReport();
				  dfg_UserErrorReportF(&token.pos, "invalid char '%c' in input", (char)invalid);
				  misc_FinishUserErrorReport();
				} else {
				  misc_StartUserErrorReport();
				  dfg_UserErrorReportF(&token.pos, "invalid char '\\x%02X' in input", (char)invalid);
				  misc_FinishUserErrorReport();
				}
				c = input_get();
				continue;
			}
			input.c = c;
			/* Reset the text buffer. */
			input.size = 0;
			return;
		}
		}
	}
}

void lexer_init(void)
{
	/* Initialize the text buffer. */
	input.capacity = 16;
	input.text     = MALLOCNZ(char, input.capacity);

	for (size_t i = '0'; i <= '9'; ++i) {
		char_class[i] |= CHAR_DIGIT;
	}
	for (size_t i = 'A'; i <= 'Z'; ++i) {
		char_class[i] |= CHAR_LETTER;
	}
	for (size_t i = 'a'; i <= 'z'; ++i) {
		char_class[i] |= CHAR_LETTER;
	}
	for (size_t i = 'u'; i <= 'z'; ++i) {
		char_class[i] |= CHAR_RESVAR;
	}
	char_class['_'] |= CHAR_SPECIAL;

	/* Prime the string table with keywords. */
#define KEYWORD(name) string_table_insert(#name, sizeof(#name), T_##name);
#define TOKEN(kind, text)
#include "tokens.inc"
#undef TOKEN
#undef KEYWORD
}
