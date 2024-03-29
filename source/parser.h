#ifndef _PARSER_H
#define _PARSER_H

#include <stdbool.h>
#include <stdint.h>

#include "tokenizer.h"

enum ParseNodeTypes {
    N_ROOT,
    N_FUNC_DEF,
    N_VAR_DEF,
    N_ARR_DEF,
    N_FUNC_CALL,
    N_BIN_OP,
    N_UN_OP,
    N_NUMBER,
    N_STRING,
    N_VARIABLE,
    N_IF,
    N_WHILE,
    N_COMPOUND,
    N_RETURN,
#ifdef DEBUG
    N_DEBUG
#endif
};

enum BinOpNodeType {
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_EQUAL,
    BINOP_LESS,
    BINOP_LEQUAL,
    BINOP_GREATER,
    BINOP_GEQUAL,
    BINOP_BITAND,
    BINOP_BITOR,
    BINOP_SHLEFT,
    BINOP_SHRIGHT,
    BINOP_ASSIGN,
};

enum UnOpNodeType {
    UNOP_NEGATE,
    UNOP_DEREF,
    UNOP_GET_ADDR,
};

extern char* bin_op_node_type_to_string[];
extern char* un_op_node_type_to_string[];

typedef struct ParseNode ParseNode;

typedef struct RootNode {
    int64_t count;
    ParseNode** definitions;
} RootNode;

typedef struct FuncDefNode {
    char* name;
    ParseNode* statement;
    size_t param_count;
    char** params;
} FuncDefNode;

typedef struct VarDefNode {
    char* name;
    ParseNode* initial_val;
} VarDefNode;

typedef struct ArrDefNode {
    char* name;
    ParseNode* size;
} ArrDefNode;

typedef struct FuncCallNode {
    char* name;
    int64_t param_count;
    ParseNode** params;
} FuncCallNode;

typedef struct BinOpNode {
    enum BinOpNodeType type;
    ParseNode* left;
    ParseNode* right;
} BinOpNode;

typedef struct UnOpNode {
    enum UnOpNodeType type;
    ParseNode* operand;
} UnOpNode;

typedef struct NumberNode {
    int64_t value;
} NumberNode;

typedef struct StringNode {
    char* contents;
} StringNode;

typedef struct VariableNode {
    char* name;
} VariableNode;

// For both if statements and while loops
typedef struct ConditionalNode {
    ParseNode* condition;
    ParseNode* statement;
    ParseNode* else_statement;  // only used in if-statements
} ConditionalNode;

typedef struct CompoundStatement {
    size_t statement_amt;
    ParseNode** statements;
} CompoundStatement;

typedef struct ReturnStatement {
    ParseNode* value;
} ReturnStatement;

#ifdef DEBUG
typedef struct DebugStatement {
    int64_t number;
} DebugStatement;
#endif

struct ParseNode {
    enum ParseNodeTypes type;
    int64_t line;
    union {
        RootNode root_info;
        FuncDefNode func_def_info;
        VarDefNode var_def_info;
        ArrDefNode arr_def_info;
        FuncCallNode func_call_info;
        BinOpNode bin_operation_info;
        UnOpNode un_operation_info;
        NumberNode number_info;
        StringNode string_info;
        VariableNode variable_info;
        ConditionalNode conditional_info;
        CompoundStatement compound_info;
        ReturnStatement return_info;
#ifdef DEBUG
        DebugStatement debug_info;
#endif
    };
};

ParseNode* parse(TokenLL* tokens);
void print_AST(ParseNode* node, int64_t indent);
void free_AST(ParseNode* node);

#endif  // _PARSER_H
