#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "tokenizer.h"

void parser_advance_token(TokenLL* tokens) {
	if(tokens->current == NULL) return;
	tokens->current = tokens->current->next;
}

static void panic(char* message, int line) {
	fprintf(stderr, "Parser error on line %d: %s\n", line, message);
	exit(1);
}

void parser_expect_token_type(Token* token, int expected_type) {
	if(token->type != expected_type) {
		char buffer[100];
		snprintf(buffer, 100, "Expected token type %s, but found %s instead", token_type_to_name[expected_type], token_type_to_name[token->type]);
		panic(buffer, token->line);
	}
}

void parser_expect_keyword(Token* token, int expected_keyword) {
	parser_expect_token_type(token, T_KEYWORD);

	if(*(int*)(token->data) != expected_keyword) {
		char buffer[100];
		snprintf(buffer, 100, "Expected keyword of type %s, but found %s, instead", token_type_to_name[*(int*)token->data], token_type_to_name[expected_keyword]);
		panic(buffer, token->line);
	}
}

ParseNode* parser_get_function_call(TokenLL* tokens) {
	parser_expect_token_type(tokens->current, T_IDENTIFIER);
	int line = tokens->current->line;

	size_t name_len = strlen(tokens->current->data);
	char* name = (char*)malloc(sizeof(char) * (name_len + 1));
	strncpy(name, tokens->current->data, name_len + 1);
	parser_advance_token(tokens);

	parser_expect_token_type(tokens->current, T_LPAREN);
	parser_advance_token(tokens);

	parser_expect_token_type(tokens->current, T_NUMBER);
	int* param = (int*)malloc(sizeof(int));
	*param = *(int*)tokens->current->data;

	parser_advance_token(tokens);

	parser_expect_token_type(tokens->current, T_RPAREN);
	parser_advance_token(tokens);

	ParseNode* result = (ParseNode*)malloc(sizeof(ParseNode));
	result->type = N_FUNC_CALL;
	result->line = line;
	result->data = malloc(sizeof(void*) * 2);
	((char**)(result->data))[0] = name;
	((int**)(result->data))[1] = param;

	return result;
}

ParseNode* parser_get_statement(TokenLL* tokens) {
	ParseNode* func_call = parser_get_function_call(tokens);
	parser_expect_token_type(tokens->current, T_SEMICOLON);
	parser_advance_token(tokens);

	ParseNode* result = (ParseNode*)malloc(sizeof(ParseNode));
	result->type = N_STATEMENT;
	result->line = func_call->line;
	result->data = malloc(sizeof(void*));
	((ParseNode**)result->data)[0] = func_call;

	return result;
}

ParseNode* parser_get_function_definition(TokenLL* tokens) {
	parser_expect_keyword(tokens->current, K_FUNC);
	int line = tokens->current->line;
	parser_advance_token(tokens);
	
	parser_expect_token_type(tokens->current, T_IDENTIFIER);
	size_t identifier_len = strlen(tokens->current->data);
	char* identifier_name = (char*)malloc(sizeof(char) * (identifier_len + 1));
	strncpy(identifier_name, tokens->current->data, identifier_len);
	identifier_name[identifier_len] = '\0';
	parser_advance_token(tokens);

	parser_expect_token_type(tokens->current, T_LPAREN);
	parser_advance_token(tokens);
	parser_expect_token_type(tokens->current, T_RPAREN);
	parser_advance_token(tokens);
	parser_expect_token_type(tokens->current, T_LBRACE);
	parser_advance_token(tokens);

	ParseNode* statements[MAX_STATEMENTS_PER_FUNC];
	int statement_counter = 0;
	while(tokens->current->type == T_IDENTIFIER) {
		if(statement_counter >= MAX_STATEMENTS_PER_FUNC) {
			panic("Too many statements for one function", tokens->current->line);
		}

		statements[statement_counter++] = parser_get_statement(tokens);
	}

	parser_expect_token_type(tokens->current, T_RBRACE);
	parser_advance_token(tokens);

	ParseNode* result = (ParseNode*)malloc(sizeof(ParseNode));
	result->type = N_FUNC_DEF;
	result->line = line;
	// TODO: I do not think this is the correct size calculation:
	result->data = malloc(sizeof(void*) * (2 + statement_counter));
	((char**)result->data)[0] = identifier_name;
	
	((int**)(result->data))[1] = (int*)malloc(sizeof(int));
	*(((int**)result->data)[1]) = statement_counter;

	((ParseNode**)result->data)[2] = (ParseNode*)malloc(sizeof(ParseNode) * statement_counter);
	for(int i = 0; i < statement_counter; ++i) {
		((ParseNode**)result->data)[2][i].type = statements[i]->type;
		((ParseNode**)result->data)[2][i].line = statements[i]->line;
		((ParseNode**)result->data)[2][i].data = statements[i]->data;
		free(statements[i]);
	}

	return result;
}

ParseNode* parse(TokenLL* tokens) {
	// TODO: resizing array
	ParseNode* functions[1000];
	int functions_counter = 0;
	while(tokens->current != NULL &&
			tokens->current->type == T_KEYWORD &&
			*(int*)tokens->current->data == K_FUNC) {
		functions[functions_counter++] = parser_get_function_definition(tokens);
	}

	if(tokens->current != NULL) {
		char buffer[100];
		snprintf(buffer, 100, "Unexpected token at end of parsing: %s", token_type_to_name[tokens->current->type]);
		panic(buffer, tokens->current->line);
	}

	ParseNode* result = (ParseNode*)malloc(sizeof(ParseNode));
	result->type = N_ROOT;
	result->line = 0;
	result->data = malloc(sizeof(void*) * 2);
	// result->data = (ParseNode*)malloc(sizeof(ParseNode) * functions_counter);
	((int**)result->data)[0] = (int*)malloc(sizeof(int));
	*(((int**)result->data)[0]) = functions_counter;
	((ParseNode**)result->data)[1] = (ParseNode*)malloc(sizeof(ParseNode) * functions_counter);

	for(int i = 0; i < functions_counter; ++i) {
		((ParseNode**)result->data)[1][i].type = functions[i]->type;
		((ParseNode**)result->data)[1][i].line = functions[i]->line;
		((ParseNode**)result->data)[1][i].data = functions[i]->data;
		free(functions[i]);
	}

	return result;
}

void free_AST(ParseNode* node) {
	if(node->data != NULL) { 
		switch(node->type) {
			case N_ROOT:
				fprintf(stderr, "Freeing root node is not properly implemented\n");
				exit(1);
				free_AST(node->data);
				break;
			case N_FUNC_DEF:
				int statment_amt = *(((int**)(node->data))[1]);
				free(((char**)(node->data))[0]);

				for(int i = 0; i < statment_amt; ++i) {
					ParseNode* to_free = ((ParseNode**)node->data)[2][i].data;
					free_AST(to_free);
				}
				free(((ParseNode**)node->data)[2]);

				break;
			case N_STATEMENT:
				free_AST(((ParseNode**)(node->data))[0]);
				break;
			case N_FUNC_CALL:
				free(((char**)(node->data))[0]);
				free(((int**)(node->data))[1]);
				break;
		}
	}
	free(node);
}

void parser_print_indent(int amt) {
	for(int i = 0; i < amt; ++i) {
		printf("  ");
	}
}

void parser_print_tree(ParseNode* node, int indent) {
	switch(node->type) {
		case N_ROOT: {
			parser_print_indent(indent);
			printf("[\n");

			int function_amt = *((int**)node->data)[0];
			
			for(int i = 0; i < function_amt; ++i) {
				parser_print_indent(indent + 1);
				printf("Function definition: {\n");
				ParseNode* funcs = ((ParseNode**)node->data)[1];
				parser_print_tree(&funcs[i], indent + 2);
				parser_print_indent(indent + 1);
				printf("}\n");
			}

			parser_print_indent(indent);
			printf("]\n");
			break;
		}
		case N_FUNC_DEF: {
			parser_print_indent(indent);
			printf("Name: %s\n", ((char**)(node->data))[0]);
			parser_print_indent(indent);
			printf("Statements: [\n");
			int statement_amt = *(((int**)(node->data))[1]);
			for(int i = 0; i < statement_amt; ++i) {
				parser_print_tree(((ParseNode**)node->data)[2] + i, indent + 1);
			}
			parser_print_indent(indent);
			printf("]\n");
			break;
		}	
		case N_STATEMENT: {
			parser_print_indent(indent);
			printf("Function call: {\n");
			parser_print_tree(((ParseNode**)(node->data))[0], indent + 1);
			parser_print_indent(indent);
			printf("}\n");
			break;
		}
		case N_FUNC_CALL: {
			parser_print_indent(indent);
			printf("Identifier: %s\n", ((char**)(node->data))[0]);
			parser_print_indent(indent);
			printf("Param: %d\n", *(((int**)(node->data))[1]));
			break;
		}
	}

}