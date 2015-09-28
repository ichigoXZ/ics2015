#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>

enum {
	LEFT_R,RIGHT_R,NUM,HEX_NUM,PLUS,SUB,MUL,DIVIDE,REG,NOTYPE = 256, EQ

	/* TODO: Add more token types */
};
 
static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{"\\(", LEFT_R},					// left parentheses
	{"\\)", RIGHT_R},					// right parentheses
	{"\[1-9][0-9]*",  NUM},					// numbers
	{"\\0x\[a-b0-9]+",HEX_NUM},			//hexadecimal numbers				
	{" +",  NOTYPE},				// spaces
	{"\\+", PLUS},					// plus
	{"\\-", SUB},						// substract
	{"\\*", MUL},					// multiply
	{"\\/", DIVIDE},					// divide
	{"\\$[a-z]+",REG},				//reg_name
	{"==", EQ}						// equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules  are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret != 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain 
				 * types of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE:  break;
					case LEFT_R:  tokens[nr_token++].type = LEFT_R;
							  	  break;
				    case RIGHT_R: tokens[nr_token++].type = RIGHT_R;
  								  break;
					case PLUS:    tokens[nr_token++].type = PLUS;
								  break;
					case SUB: 	  tokens[nr_token++].type = SUB;
								  break;
				    case MUL:	  tokens[nr_token++].type = MUL;
								  break;
					case DIVIDE:  tokens[nr_token++].type = DIVIDE;
								  break;
					case NUM: tokens[nr_token].type = NUM;
							  strncpy(tokens[nr_token].str,substr_start,substr_len);		
							  nr_token++;
							  break;
					case HEX_NUM: tokens[nr_token].type = HEX_NUM;
								  strncpy(tokens[nr_token].str,substr_start,substr_len);
								  nr_token++;
								  break;
					case REG: tokens[nr_token].type = REG;
    						  strncpy(tokens[nr_token].str,substr_start,substr_len);
							  nr_token++;
							  break; 
					default: panic("please implement me");
							 assert(0);
				}

				break;
			}
		}

		if(i == NR_REGEX) {
		//	printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

uint32_t eval(uint32_t p,uint32_t q);
uint32_t dominant_operator(uint32_t p,uint32_t q);
bool check_parentheses(uint32_t p,uint32_t q);

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
   	//panic("please implement me");
	
	return eval(0,nr_token-1);
}

uint32_t eval(uint32_t p,uint32_t q){
	if(p > q){
		printf("Bad expression./n");
		assert(0);}
	else if(p == q){ 		//single token
		if(tokens[p].type == NUM)
			return (uint32_t)atoi(tokens[p].str);
		else if(tokens[p].type == HEX_NUM)
			return (uint32_t)strtol(tokens[p].str,NULL,16);
		else if(tokens[p].type == REG){
			if(strcmp(tokens[p].str+1,"eax")==0)
				return cpu.eax;
			else if(strcmp(tokens[p].str+1,"ecx")==0)
				return cpu.ecx;
			else if(strcmp(tokens[p].str+1,"edx")==0)
				return cpu.edx;
			else if(strcmp(tokens[p].str+1,"ebx")==0)
				return cpu.ebx;
			else if(strcmp(tokens[p].str+1,"esp")==0)
				return cpu.esp;
			else if(strcmp(tokens[p].str+1,"ebp")==0)
				return cpu.ebp;
			else if(strcmp(tokens[p].str+1,"esi")==0)
				return cpu.esi;
			else if(strcmp(tokens[p].str+1,"edi")==0)
				return cpu.edi;
			else if(strcmp(tokens[p].str+1,"eip")==0)
				return cpu.eip;
			else 
				assert(0);}
		else
			assert(0);
				}
	else if(check_parentheses(p,q) == true)		
		return eval(p+1,q-1);
	else{
		uint32_t op = dominant_operator(p,q);
		uint32_t val1 = eval(p,op-1);
  	    uint32_t val2 = eval(op+1,q);

		switch(tokens[op].type){
			case PLUS: return val1 + val2;
			case SUB:  return val1 - val2;
			case MUL:  return val1 * val2;
			case DIVIDE: if(!val2){
						    assert(0);}	
    					 else              
    					    return val1 / val2;
			default: printf("not surported");
					 assert(0);}
	}
	return 0;
}

uint32_t dominant_operator(uint32_t p,uint32_t q){
	int  pos =(int) p;
	int  i = 0,j = 0;
	typedef struct mark{
		int position;
		int op;
	}Mark;
	Mark stack[32];
	for( ;pos <= q;pos++)
		switch(tokens[pos].type){
			case PLUS : stack[i].position = pos;
						stack[i++].op = 1;
						break;
		    case SUB  : stack[i].position = pos;
						stack[i++].op = 1;
						break;
			case MUL  : stack[i].position = pos;
						stack[i++].op = 0;
						break;
			case DIVIDE:stack[i].position = pos;
						stack[i++].op = 0; 
						break;
			case LEFT_R:
						 while(tokens[pos].type != RIGHT_R && pos<q)
					   	{
							pos++;
						}
						 break;
			default: break;
			}
	for(j = i-1 ;j >= 0;j--)
		if(stack[j].op){
			return stack[j].position;}
	return stack[i-1].position;
}

bool check_parentheses(uint32_t p, uint32_t q){
	int flag = 1;
	uint32_t pos = p+1;
	if(tokens[p].type != LEFT_R)
		return false;
	else{
		for( ; pos<q; pos++){
			if(tokens[pos].type == LEFT_R)
				flag++;
			if(tokens[pos].type == RIGHT_R){
				if(flag) flag--;
				else 
					return false;}
			if(!flag)
				return false;
		}
		if(tokens[q].type == RIGHT_R)
			return true;
		else
			return false;
	}
}

