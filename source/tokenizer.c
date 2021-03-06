#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helperfunctions.h"

static int curr_line = 1;

char* token_type_to_name[] = {
    "T_HEAD",
    "T_NUMBER",
    "T_IDENTIFIER",
    "T_KEYWORD",
    "T_LPAREN",
    "T_RPAREN",
    "T_LBRACE",
    "T_RBRACE",
    "T_LSQUARE",
    "T_RSQUARE",
    "T_SEMICOLON",
    "T_ASSIGN",
    "T_PLUS",
    "T_MINUS",
    "T_ASTERISK",
    "T_SLASH",
    "T_GREATER",
    "T_LESS",
    "T_GEQUAL",
    "T_LEQUAL",
    "T_EQUAL",
    "T_COMMA",
    "T_AMPERSAND",
    "T_AT",
    "T_STRING",
};

char* keyword_type_to_name[] = {
    "K_FUNC",
    "K_VAR",
    "K_IF",
    "K_ELSE",
    "K_WHILE",
    "K_RETURN",
#ifdef DEBUG
    "T_DEBUG",
#endif
};

static void panic(char* message) {
    fprintf(stderr, "Error while tokenizing on line %d: %s\n", curr_line, message);
    exit(1);
}

static void append_token(TokenLL* tokens, int new_type, void* data) {
    Token* new_token = (Token*)malloc(sizeof(Token));
    new_token->type = new_type;
    new_token->next = NULL;
    new_token->line = curr_line;
    switch (new_type) {
        case T_NUMBER:
        case T_KEYWORD:
            new_token->number = *(int*)data;
            break;
        case T_IDENTIFIER:
        case T_STRING:
            new_token->string = (char*)data;
            break;
    }

    tokens->tail->next = new_token;
    tokens->tail = new_token;
}

static void create_number(char** text, TokenLL* tokens) {
    char buffer[MAX_IDENTIFIER_LENGTH] = {0};
    size_t buffer_i = 0;
    while (is_number(**text)) {
        if (buffer_i >= MAX_IDENTIFIER_LENGTH - 2) break;
        buffer[buffer_i++] = **text;
        ++*text;
    }
    buffer[buffer_i] = '\0';

    int result = atoi(buffer);

    append_token(tokens, T_NUMBER, &result);
}

static void create_identifier(char** text, TokenLL* tokens) {
    char buffer[MAX_IDENTIFIER_LENGTH] = {0};
    size_t buffer_i = 0;
    while (is_identifier_char(**text)) {
        if (buffer_i >= MAX_IDENTIFIER_LENGTH - 2) break;
        buffer[buffer_i++] = **text;
        ++*text;
    }
    buffer[buffer_i] = '\0';

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
    char* data = (char*)malloc(sizeof(char) * (buffer_i + 1));
    strncpy(data, buffer, buffer_i + 1);
    data[buffer_i] = '\0';
#pragma GCC diagnostic pop

    append_token(tokens, T_IDENTIFIER, data);
}

static void check_identifier_is_keyword(Token* token) {
    if (token->type != T_IDENTIFIER) return;

    if (strcmp(token->string, "func") == 0) {
        free(token->string);
        token->type = T_KEYWORD;
        token->number = K_FUNC;
    } else if (strcmp(token->string, "var") == 0) {
        free(token->string);
        token->type = T_KEYWORD;
        token->number = K_VAR;
    } else if (strcmp(token->string, "if") == 0) {
        free(token->string);
        token->type = T_KEYWORD;
        token->number = K_IF;
    } else if (strcmp(token->string, "else") == 0) {
        free(token->string);
        token->type = T_KEYWORD;
        token->number = K_ELSE;
    } else if (strcmp(token->string, "while") == 0) {
        free(token->string);
        token->type = T_KEYWORD;
        token->number = K_WHILE;
    } else if (strcmp(token->string, "return") == 0) {
        free(token->string);
        token->type = T_KEYWORD;
        token->number = K_RETURN;
    }
#ifdef DEBUG
    else if (strcmp(token->string, "debug") == 0) {
        free(token->string);
        token->type = T_KEYWORD;
        token->number = K_DEBUG;
    }
#endif
}

static bool should_skip(char c) {
    switch (c) {
        case ' ':
        case '\n':
        case '\r':
        case '\t':
            return true;
        default:
            return false;
    }
}

void tokenize(char* text, TokenLL** result) {
    *result = (TokenLL*)malloc(sizeof(TokenLL));

    Token* head = (Token*)malloc(sizeof(Token));
    head->type = T_HEAD;
    head->next = NULL;

    (*result)->head = head;
    (*result)->tail = head;

    while (*text != EOF) {
        if (is_number(*text)) {
            create_number(&text, *result);
        } else if (is_identifier_char(*text)) {
            create_identifier(&text, *result);
            check_identifier_is_keyword((*result)->tail);
        } else if (*text == '(') {
            append_token(*result, T_LPAREN, NULL);
            ++text;
        } else if (*text == ')') {
            append_token(*result, T_RPAREN, NULL);
            ++text;
        } else if (*text == '{') {
            append_token(*result, T_LBRACE, NULL);
            ++text;
        } else if (*text == '}') {
            append_token(*result, T_RBRACE, NULL);
            ++text;
        } else if (*text == '[') {
            append_token(*result, T_LSQUARE, NULL);
            ++text;
        } else if (*text == ']') {
            append_token(*result, T_RSQUARE, NULL);
            ++text;
        } else if (*text == ';') {
            append_token(*result, T_SEMICOLON, NULL);
            ++text;
        } else if (*text == ',') {
            append_token(*result, T_COMMA, NULL);
            ++text;
        } else if (*text == '=') {
            ++text;
            if (*text != EOF && *text == '=') {
                append_token(*result, T_EQUAL, NULL);
                ++text;
            } else {
                append_token(*result, T_ASSIGN, NULL);
            }
        } else if (*text == '+') {
            append_token(*result, T_PLUS, NULL);
            ++text;
        } else if (*text == '-') {
            append_token(*result, T_MINUS, NULL);
            ++text;
        } else if (*text == '*') {
            append_token(*result, T_ASTERISK, NULL);
            ++text;
        } else if (*text == '@') {
            append_token(*result, T_AT, NULL);
            ++text;
        } else if (*text == '&') {
            append_token(*result, T_AMPERSAND, NULL);
            ++text;
        } else if (*text == '|') {
            append_token(*result, T_PIPE, NULL);
            ++text;
        } else if (*text == '/') {
            ++text;
            if (*text == '/') {
                ++text;
                while (*text != '\n' && *text != EOF) {
                    ++text;
                }
            } else {
                append_token(*result, T_SLASH, NULL);
            }
        } else if (*text == '>') {
            ++text;
            if (*text != EOF && *text == '=') {
                ++text;
                append_token(*result, T_GEQUAL, NULL);
            } else if (*text != EOF && *text == '>') {
                ++text;
                append_token(*result, T_DBL_GREATER, NULL);
            } else {
                append_token(*result, T_GREATER, NULL);
            }
        } else if (*text == '<') {
            ++text;
            if (*text != EOF && *text == '=') {
                ++text;
                append_token(*result, T_LEQUAL, NULL);
            } else if (*text != EOF && *text == '<') {
                ++text;
                append_token(*result, T_DBL_LESS, NULL);
            } else {
                append_token(*result, T_LESS, NULL);
            }
        } else if (*text == '"') {
            ++text;
            char* start = text;
            while (*text != EOF && *text != '"' && *text != '\n') {
                ++text;
            }

            if (*text != '"') {
                panic("No closing quote found for string literal");
            }

            char* end = text;
            size_t length = end - start;
            ++text;

            char* contents = malloc(sizeof(char) * length + 1);
            strncpy(contents, start, length);
            contents[length] = '\0';

            append_token(*result, T_STRING, contents);

        } else if (should_skip(*text)) {
            if (*text == '\n') ++curr_line;
            ++text;
        } else {
            char buffer[100];
            snprintf(buffer, 100, "Unexpected char: '%c'", *text);
            panic(buffer);
        }
    }

    (*result)->head = head->next;
    (*result)->current = (*result)->head;
    free(head);
}

void print_tokens(Token* node) {
    if (node == NULL) {
        printf("\n");
        return;
    }

    switch (node->type) {
        case T_HEAD:
            printf("HEAD -> ");
            break;
        case T_IDENTIFIER:
            printf("IDENTIFIER: %s -> ", node->string);
            break;
        case T_KEYWORD:
            printf("KEYWORD: %s -> ", keyword_type_to_name[node->number]);
            break;
        case T_NUMBER:
            printf("NUMBER: %d -> ", node->number);
            break;
        case T_LPAREN:
            printf("'(' -> ");
            break;
        case T_RPAREN:
            printf("')' -> ");
            break;
        case T_LBRACE:
            printf("'{' -> ");
            break;
        case T_RBRACE:
            printf("'}' -> ");
            break;
        case T_LSQUARE:
            printf("'[' -> ");
            break;
        case T_RSQUARE:
            printf("']' -> ");
            break;
        case T_SEMICOLON:
            printf("';' -> ");
            break;
        case T_ASSIGN:
            printf("'=' -> ");
            break;
        case T_PLUS:
            printf("'+' -> ");
            break;
        case T_MINUS:
            printf("'-' -> ");
            break;
        case T_SLASH:
            printf("'/' -> ");
            break;
        case T_ASTERISK:
            printf("'*' -> ");
            break;
        case T_EQUAL:
            printf("'==' -> ");
            break;
        case T_COMMA:
            printf("',' -> ");
            break;
        case T_LESS:
            printf("'<' -> ");
            break;
        case T_LEQUAL:
            printf("'<=' -> ");
            break;
        case T_GREATER:
            printf("'>' -> ");
            break;
        case T_GEQUAL:
            printf("'>=' -> ");
            break;
        case T_AMPERSAND:
            printf("'&' -> ");
            break;
        case T_AT:
            printf("'@' -> ");
            break;
        case T_PIPE:
            printf("'|' -> ");
            break;
        case T_DBL_GREATER:
            printf("'>>' -> ");
            break;
        case T_DBL_LESS:
            printf("'<<' -> ");
            break;
        case T_STRING:
            printf("STRING: \"%s\" ->", node->string);
            break;
        default:
            assert(false && "Unexhaustive switch for printing tokens");
    }
    print_tokens(node->next);
}

void free_token(Token* node) {
    if (node == NULL) return;
    free_token(node->next);
    if (node->type == T_IDENTIFIER || node->type == T_STRING) {
        free(node->string);
    }
    free(node);
}