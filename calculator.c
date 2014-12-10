#include "stdio.h"
#include "stdlib.h"

enum OperatorType
{
	ADD,
	SUB,
	DIV,
	MULT,
	LPAR,
	RPAR,
};

typedef struct OperatorNode OperatorNode;
typedef struct OperandNode OperandNode;
typedef struct OperatorStack OperatorStack;
typedef struct OperandStack OperandStack;

struct OperatorNode
{
	unsigned int m_type;
	OperatorNode* m_pNext;
};

struct OperandNode
{
	float m_fValue;
	OperandNode* m_pNext;
};

struct OperatorStack
{
	OperatorNode* m_pHead;
};

struct OperandStack
{
	OperandNode* m_pHead;
};

int isOperator(char character)
{
	if(	character == '+' || 
		character == '-' || 
		character == '*' || 
		character == '/' ||
		character == '^')
		return 1;
	return 0;
}

int isNumeric(char character)
{
	if( character - '0' < 10 && character - '0' >= 0 )
		return 1;
	return 0;
}

int precedence(int type)
{
	if(type == ADD || type == SUB)
		return 1;
	if(type == DIV || type == MULT)
		return 2;
	if(type == LPAR)
		return -1;
}

float operate(float left, int type ,float right)
{
	if(type == ADD)
		return left + right;
	if(type == SUB)
		return left - right;
	if(type == MULT)
		return left * right;
	if(type == DIV)
		return left / right;
}

//#define DEBUG

#ifdef DEBUG
char typeToChar(int type)
{
	if(type == ADD)
		return '+';
	if(type == SUB)
		return '-';
	if(type == MULT)
		return '*';
	if(type == DIV)
		return '/';
	if(type == LPAR)
		return '(';
	if(type == RPAR)
		return ')';
}

#endif

float eval(char* expression)
{
	OperatorStack opStack;
	OperandStack opandStack;
	float leftOperandValue, returnVal;
	int i;
	int currentTotal = 0; //Used to keep track of number being read. It can take multiple characters.
	int opType;
	int readingOperand = 0;

	opStack.m_pHead = 0;
	opandStack.m_pHead = 0;

	for(i = 0; expression[i] != '\0'; i++)
	{
#ifdef DEBUG
		OperatorNode* curOpNode;
		OperandNode* curAndNode;
		
		curOpNode = opStack.m_pHead;
		curAndNode = opandStack.m_pHead;
		printf("Iteration %i\n",i);
		printf("Operator Stack:\n");
		while(curOpNode != 0)
		{
			printf("%c\n",typeToChar(curOpNode->m_type));
			curOpNode = curOpNode->m_pNext;
		}

		printf("Operand Stack:\n");
		while(curAndNode != 0)
		{
			printf("%f\n",curAndNode->m_fValue);
			curAndNode = curAndNode->m_pNext;
		}
#endif		
		if( isOperator(expression[i]) )
		{
			if(readingOperand == 1)
			{
				readingOperand = 0;
				leftOperandValue = currentTotal;
				if(currentTotal > 0)
					currentTotal = 0;
			
				//Push Operand
				OperandNode* newNode = malloc(sizeof( struct OperandNode  ));
				newNode->m_fValue = leftOperandValue;
				newNode->m_pNext = opandStack.m_pHead;
				opandStack.m_pHead = newNode;
			}

			//Gather the Operator Type:
			if(expression[i] == '+')
				opType = ADD;
			else if(expression[i] == '-')
				opType = SUB;
			else if(expression[i] == '*')
				opType = MULT;
			else if(expression[i] == '(')
				opType = LPAR;
			else if(expression[i] == ')')
				opType = RPAR;
			else
				opType = DIV;


			//If Operator is of lower or equal precedence than the current stack head, do stack head.
			while(opStack.m_pHead != 0 && precedence(opStack.m_pHead->m_type) >= precedence(opType) )
			{
				//Pop the two Operands.
				float left, right;
				int popedOp;
				OperandNode* toDelete;
				OperatorNode* toDeleteOp;

				if(opandStack.m_pHead == 0 || opandStack.m_pHead->m_pNext == 0)
				{
					printf("ERROR: NOT ENOUGH OPERANDS!\n");
					return -1.0;
				}
				right = opandStack.m_pHead->m_fValue;
				left = opandStack.m_pHead->m_pNext->m_fValue;
				toDelete = opandStack.m_pHead;

				//Delete 1 operand node here. We'll reuse the second for the result.
				opandStack.m_pHead = opandStack.m_pHead->m_pNext;
				free(toDelete);				
				
				//Pop the Operator.
				popedOp = opStack.m_pHead->m_type;
				toDeleteOp = opStack.m_pHead;
				opStack.m_pHead = opStack.m_pHead->m_pNext;
				free(toDeleteOp);
				
				//Perform Operation and Push result.
				if(popedOp == DIV && right == 0)
				{
					printf("ERROR!: Division by 0!\n");
					return -1.0;
				}
				opandStack.m_pHead->m_fValue = operate(left,popedOp,right);
			}
			
			//Just push the Operator
			OperatorNode* newOpNode = malloc(sizeof( struct OperatorNode ));
			newOpNode->m_type = opType;
			newOpNode->m_pNext = opStack.m_pHead;
			opStack.m_pHead = newOpNode;
				
		}
		else if( expression[i] == '(' )
		{
			// Just push into opStack
			OperatorNode* newParNode = malloc(sizeof( struct OperatorNode));
			newParNode->m_type = LPAR;
			newParNode->m_pNext = opStack.m_pHead;
			opStack.m_pHead = newParNode;
		}
		else if( expression[i] == ')')
		{
			if(opStack.m_pHead == 0)
			{
				printf("ERROR!: Parenthesis Mitchmatch!\n");
				return -1.0;
			}

			//If we were reading an operand, add it here.			
			if(readingOperand == 1)
			{
				readingOperand = 0;
				leftOperandValue = currentTotal;
				if(currentTotal > 0)
					currentTotal = 0;
			
				//Push Operand
				OperandNode* newNode = malloc(sizeof( struct OperandNode  ));
				newNode->m_fValue = leftOperandValue;
				newNode->m_pNext = opandStack.m_pHead;
				opandStack.m_pHead = newNode;
			}

			//evaluate until we find closing opening parenthesis
			while(opStack.m_pHead->m_type != LPAR)
			{
				//Pop the two Operands.
				float left, right;
				int popedOp;
				OperandNode* toDelete;
				OperatorNode* toDeleteOp;

				if(opandStack.m_pHead == 0 || opandStack.m_pHead->m_pNext == 0)
				{
					printf("ERROR: NOT ENOUGH OPERANDS!\n");
					return -1.0;
				}
				right = opandStack.m_pHead->m_fValue;
				left = opandStack.m_pHead->m_pNext->m_fValue;
				toDelete = opandStack.m_pHead;

				//Delete 1 operand node here. We'll reuse the second for the result.
				opandStack.m_pHead = opandStack.m_pHead->m_pNext;
				free(toDelete);				
				
				//Pop the Operator.
				popedOp = opStack.m_pHead->m_type;
				toDeleteOp = opStack.m_pHead;
				opStack.m_pHead = opStack.m_pHead->m_pNext;
				free(toDeleteOp);
				
				//Perform Operation and Push result.
				if(popedOp == DIV && right == 0)
				{
					printf("ERROR!: Division by 0!\n");
					return -1.0;
				}
				opandStack.m_pHead->m_fValue = operate(left,popedOp,right);
				
				if(opStack.m_pHead == 0)
				{
					printf("ERROR!: PARENTHESIS MITCHMATCH!\n");
					return -1.0;
				}
			}

			//Remove the LPAR Op from stack here.
			OperatorNode* opToDelete;
			opToDelete = opStack.m_pHead;
			opStack.m_pHead = opStack.m_pHead->m_pNext;
			free(opToDelete);
		}
		else if( isNumeric(expression[i]))
		{
			readingOperand = 1;
			currentTotal = 10*currentTotal + (expression[i] - '0');
		}
		else
		{
			printf("Syntax Error: Found non-accepted character: %c\n",expression[i]);	
			return -1.0;
		}			
	}
	
	if(readingOperand == 1)
	{
		//Push our remaining operand	
		OperandNode* newNode = malloc(sizeof( struct OperandNode  ));
		newNode->m_fValue = currentTotal;
		newNode->m_pNext = opandStack.m_pHead;
		opandStack.m_pHead = newNode;
	}
#ifdef DEBUG
		OperatorNode* curOpNode;
		OperandNode* curAndNode;
		
		curOpNode = opStack.m_pHead;
		curAndNode = opandStack.m_pHead;
		printf("LAST EVALUATION INCOMING \n",i);
		printf("Operator Stack:\n");
		while(curOpNode != 0)
		{
			printf("%c\n",typeToChar(curOpNode->m_type));
			curOpNode = curOpNode->m_pNext;
		}

		printf("Operand Stack:\n");
		while(curAndNode != 0)
		{
			printf("%f\n",curAndNode->m_fValue);
			curAndNode = curAndNode->m_pNext;
		}
#endif		
	//Evaluate the rest of our stack.	
	while(opStack.m_pHead != 0 )
	{
		//Pop the two Operands.
		float left, right;
		int popedOp;
		OperandNode* toDelete;
		OperatorNode* toDeleteOp;

		if(opandStack.m_pHead == 0 || opandStack.m_pHead->m_pNext == 0)
		{
			printf("ERROR: NOT ENOUGH OPERANDS!\n");
			return -1.0;
		}
		right = opandStack.m_pHead->m_fValue;
		left = opandStack.m_pHead->m_pNext->m_fValue;
		toDelete = opandStack.m_pHead;

		//Delete 1 operand node here. We'll reuse the second for the result.
		opandStack.m_pHead = opandStack.m_pHead->m_pNext;
		free(toDelete);				
				
		//Pop the Operator.
		popedOp = opStack.m_pHead->m_type;
		toDeleteOp = opStack.m_pHead;
		opStack.m_pHead = opStack.m_pHead->m_pNext;
		free(toDeleteOp);
		
		//Perform Operation and Push result.
		if(popedOp == DIV && right == 0)
		{
			printf("ERROR!: Division by 0!\n");
			return -1.0;
		}
		if( popedOp == LPAR || popedOp == RPAR)
		{
			printf("ERROR!: MISMATCHING PARENTHESIS!\n");
			return -1.0;
		}
		opandStack.m_pHead->m_fValue = operate(left,popedOp,right);
	}
	
	if(opandStack.m_pHead == 0 || opandStack.m_pHead->m_pNext != 0)
	{
		printf("ERROR! TOO MANY OR TOO FEW OPERANDS LEFT!\n");
		return -1.0;
	}
	
	//Clear our remaining operand and return its value.
	returnVal = opandStack.m_pHead->m_fValue;
	free(opandStack.m_pHead);
	return returnVal;
}

int main()
{
	char input[128];
	while(1)
	{
		printf("Please Enter Something To Calculate.\n");
		scanf("%s",&input);
		printf("%f\n",eval(input));
	}
}
