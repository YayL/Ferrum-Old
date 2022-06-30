#include "token.h"

#include "common.h"

struct Token * init_token(char* value, int type) {

	struct Token * token = malloc(sizeof(struct Token));
	token->value = value;
	token->type = type;

	return token;
}

const char* token_type_to_str(int type) {
	switch(type) {
		case TOKEN_ID: return "TOKEN_ID";
		case TOKEN_TYPE: return "TOKEN_TYPE";
		case TOKEN_DEF: return "TOKEN_DEF";
		case TOKEN_SEMI: return "TOKEN_SEMI";
		case TOKEN_LPAREN: return "TOKEN_LPAREN";
		case TOKEN_RPAREN: return "TOKEN_RPAREN";
		case TOKEN_LBRACE: return "TOKEN_LBRACE";
		case TOKEN_RBRACE: return "TOKEN_RBRACE";
		case TOKEN_LBRACKET: return "TOKEN_LBRACKET";
		case TOKEN_RBRACKET: return "TOKEN_RBRACKET";
		case TOKEN_COLON: return "TOKEN_COLON";
		case TOKEN_COMMA: return "TOKEN_COMMA";
		case TOKEN_CALL: return "TOKEN_CALL";
		case TOKEN_EQUALS: return "TOKEN_EQUALS";
		case TOKEN_LT: return "TOKEN_LT";
		case TOKEN_GT: return "TOKEN_GT";
		case TOKEN_INT: return "TOKEN_INT";
		case TOKEN_STRING: return "TOKEN_STRING";
		case TOKEN_OP: return "TOKEN_OP";
		case TOKEN_COMMENT: return "TOKEN_COMMENT";
		case TOKEN_EOF: return "TOKEN_EOF";
	}
	return "UNDEFINED";
}

char* token_to_str(struct Token * token) {
	const char* type_str = token_type_to_str(token->type);
	const char* template = "<type='%s', code=%d, value='%p'>";
	const size_t size = strlen(type_str) + strlen(template) + 10;

	char * str = malloc(size);
	snprintf(str, size, template, type_str, token->type, token->value);
	str[size - 1] = 0;

	return str; 
} 