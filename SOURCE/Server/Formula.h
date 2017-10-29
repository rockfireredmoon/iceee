#include "stdarg.h"
#include "CommonTypes.h"
#include "StringList.h"
#include <math.h>

enum OperatorType
{
	OP_NONE = 0,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_MOD,
	OP_NOT,
	OP_EQUALS,
};

using namespace Ability2;

class Formula
{
public:
	static void PreparePostfix(const STRINGLIST &inputTokens, STRINGLIST &resultOutput)
	{
		STRINGLIST stack;
		resultOutput.clear();
		//http://en.wikipedia.org/wiki/Shunting_yard_algorithm
		for(size_t i = 0; i < inputTokens.size(); i++)
		{
			if(isNumber(inputTokens[i]))
				resultOutput.push_back(inputTokens[i]);
			else if(isVariable(inputTokens[i]))
				resultOutput.push_back(inputTokens[i]);
			else if(isOperator(inputTokens[i]))
			{
				const char op1 = inputTokens[i][0];
				while(peekTopOperator(stack) != 0)
				{
					const char op2 = peekTopOperator(stack);
					bool pop = false;
					if(isLeftAssociative(op1) && (getOperatorPrecedent(op1) <= getOperatorPrecedent(op2)))
						pop = true;
					else if(getOperatorPrecedent(op1) < getOperatorPrecedent(op2))
						pop = true;
					if(pop == true)
					{
						resultOutput.push_back(stack.back());
						stack.pop_back();
					}
					else
						break;
				}
				stack.push_back(inputTokens[i]);
			}
			else if(isLeftParenthesis(inputTokens[i]))
			{
				stack.push_back(inputTokens[i]);
			}
			else if(isRightParenthesis(inputTokens[i]))
			{
				while(stack.size() > 0)
				{
					if(isLeftParenthesis(stack.back()) == false)
					{
						resultOutput.push_back(stack.back());
						stack.pop_back();
					}
					else
					{
						stack.pop_back();
						break;
					}
				}
				if(stack.size() > 0)
				{
					if(isRightParenthesis(stack.back()))
					{
						resultOutput.push_back(stack.back());
						stack.pop_back();
					}
				}
			}
		}
		while(stack.size() > 0)
		{
			if(isLeftParenthesis(stack.back()))
			{
				break;
			}
			else
			{
				resultOutput.push_back(stack.back());
				stack.pop_back();
			}
		}
	}

	//Detect any cases where a negative sign or multiplication should implicitly associate with
	//when immediately preceeding a left parenthesis.  This may expand the token list adjusting
	//an empty sign to "-1" and inserting a "*" token.
	static void ExpandAssociativeTokens(STRINGLIST &operatorTokens, AbilityVerify *verify)
	{
		bool associateMultiplication = false;
		for(size_t i = 0; i < operatorTokens.size(); i++)
		{
			if(i > 0 && isLeftParenthesis(operatorTokens[i]) == true)
			{
				if(operatorTokens[i - 1].compare("-") == 0)
				{
					DebugLog(verify, "[WARNING] Implicit association of negative sign and left parenthesis Ex:{-(2)}");
					operatorTokens[i - 1] = "-1";
					associateMultiplication = true;
				}
				else if(isVariable(operatorTokens[i - 1]) == true)
				{
					DebugLog(verify, "[WARNING] Implicit multiplication between variable and left parenthesis. Ex:{var(2)}");
					associateMultiplication = true;
				}
				else if(isNumber(operatorTokens[i - 1]) == true)
				{
					DebugLog(verify, "[WARNING] Implicit multiplication between numerical value and left parenthesis. Ex:{1(2)}");
					associateMultiplication = true;
				}

				if(associateMultiplication == true)
				{
					operatorTokens.insert(operatorTokens.begin() + i, "*");
					associateMultiplication = false;
				}
			}
		}
	}

	//Verify that all variable identifier tokens in an expression's tokenized operator list can be resolved by the ability system.
	static void VerifyVariableNames(const STRINGLIST &operatorTokens, AbilityManager2 *symbolResolver, AbilityVerify *verify)
	{
		if(symbolResolver == NULL)
			return;

		for(size_t i = 0; i < operatorTokens.size(); i++)
		{
			if(isVariable(operatorTokens[i]) == true)
			{
				if(symbolResolver != NULL)
				{
					if(symbolResolver->CheckValidVariableName(operatorTokens[i]) == false)
						DebugLog(verify, "Formula token is not a recognized variable: [%s]", operatorTokens[i].c_str());
				}
			}
		}
	}

	static double TestEvaluate(const STRINGLIST &postfixTokens, AbilityVerify *verify)
	{
		return Evaluate(postfixTokens, NULL, verify);
	}
	static double Evaluate(const STRINGLIST &postfixTokens, AbilityManager2 *symbolResolver, AbilityVerify *verify)
	{
		if(postfixTokens.size() == 0)
		{
			DebugLog(verify, "No postfix expression on the stack.");
			return 0.0;
		}
		STRINGLIST stack;
		char convBuffer[64];
		for(size_t i = 0; i < postfixTokens.size(); i++)
		{
			if(isNumber(postfixTokens[i]))
				stack.push_back(postfixTokens[i]);
			else if(isVariable(postfixTokens[i]) == true)
			{
				sprintf(convBuffer, "%g", ResolveValue(postfixTokens[i], symbolResolver, false));
				stack.push_back(convBuffer);
			}
			else if(isOperator(postfixTokens[i]) == true)
			{
				const char op = postfixTokens[i][0];
				size_t count = getArgCount(op);
				if(stack.size() < count)
				{
					DebugLog(verify, "Not enough values for operator [%s] (needs %d operands, has %d).", postfixTokens[i].c_str(), count, stack.size());
					return 0.0;
				}
				double left = 0.0F;
				double right = 0.0F;
				double result = 0.0F;
				switch(count)
				{
				case 2:
					right = ResolveValue(stack.back(), symbolResolver, false);
					stack.pop_back();
					left = ResolveValue(stack.back(), symbolResolver, false);
					stack.pop_back();
					break;
				case 1:
					left = ResolveValue(stack.back(), symbolResolver, false);
					stack.pop_back();
					break;
				}
				switch(op)
				{
				case '+': result = left + right; break;
				case '-': result = left - right; break;
				case '*': result = left * right; break;
				case '/':
					if(DoubleEquivalent(right, 0.0) == true)
					{
						DebugLog(verify, "ERROR: Divide by zero when evaluating expression.");
						result = 0.0;
					}
					else
					{
						result = left / right;
					}
					break;
				case '%': result = (int)left % (int)right; break;
				case '!': result = !left; break;
				}
				sprintf(convBuffer, "%g", result);
				stack.push_back(convBuffer);
			}
		}
		if(stack.size() == 1)
			return atof(stack.back().c_str());

		DebugLog(verify, "Too many values in the expression.");
		return 0.0;
	}
	static void TokenizeExpression(const std::string &expressionString, STRINGLIST &outputTokenList)
	{
		const char *separator = "+-*/()!%, \t";
		// TODO - Em - this was unused - see if its needed
//		const int sepLen = strlen(separator);

//		size_t lastParen = 0;
		size_t len = expressionString.length();
		std::string extract;
		// TODO - Em - this was unused - see if its needed
//		bool lastTokenOperator = false;

		for(size_t spos = 0; spos < len; spos++)
		{
			size_t epos = expressionString.find_first_of(separator, spos);
			if(epos == std::string::npos)
				epos = len;
			else
			{
				//Hack for negative numbers.  Extend the end position of this token to include the
				//rest of the number, if applicable.
				if(expressionString[epos] == '-')
				{
					if(isNegativeNumber(expressionString, epos) == true)
					{
						epos = expressionString.find_first_of(separator, epos + 1);
						if(epos == std::string::npos)
							epos = len;
					}
				}
			}

			//Check a single character tokens.  Discard whitespace and extend the end position
			//so the substring will copy at least 1 character.
			if(spos == epos)
			{
				if(expressionString[spos] == ' ' || expressionString[spos] == '\t')
					continue;
				epos++;
			}

			extract = expressionString.substr(spos, epos - spos);
			// TODO - Em - this was unused
//			lastTokenOperator = isOperator(extract); //Need this for the negative number hack.
			outputTokenList.push_back(extract);

			spos += (epos - spos) - 1;
			if(epos >= len)
				break;
		}
	}
	static void DebugPrintTokens(const char *label, const STRINGLIST &tokenList)
	{
		printf("%s:\n", label);
		for(size_t i = 0; i < tokenList.size(); i++)
			printf("[%zu]=%s\n", i, tokenList[i].c_str());
	}

private:
	//Print a variable argument printf() style message to the required output, or pass it
	//to a verification error message collector if the routine is being pre-verified before
	//normal operating runtime conditions.
	static void DebugLog(AbilityVerify *verify, const char *format, ...)
	{
		va_list args;
		va_start (args, format);
		if(verify != NULL)
			verify->AddError(format, args);
		else
			g_Log.AddMessageFormatArg(format, args);
		va_end (args);
	}

	static bool DoubleEquivalent(double left, double right)
	{
		return (fabs(left - right) <= 0.001);
	}

	static const char peekTopOperator(const STRINGLIST &stack)
	{
		if(stack.size() > 0)
		{
			if(stack.back().size() == 0)
				return 0;
			return stack.back()[0];
		}
		return 0;
	}
	static int getOperatorPrecedent(const char op)
	{
		switch(op)
		{
		case '!':
			return 4;
		case '*':
		case '/':
		case '%':
			return 3;
		case '+':
		case '-':
			return 2;
		case '=':
			return 1;
		}
		return 0;
	}
	static bool isLeftAssociative(const char op)
	{
		switch(op)
		{
		case '*':
		case '/':
		case '%':
		case '+':
		case '-':
			return true;
		case '=':
		case '!':
			return false;
		}
		return false;
	}
	static int getArgCount(const char op)
	{
		switch(op)
		{
		case '*':
		case '/':
		case '%':
		case '-':
		case '+':
		case '=':
			return 2;
		case '!':
			return 1;
		}
		return 0;
	}

	static bool isNumericChar(const char ch)
	{
		if(ch >= '0' && ch <= '9')
			return true;
		if(ch == '.')
			return true;
		if(ch == '-')
			return true;

		return false;
	}

	static bool isNumber(const std::string& token)
	{
		g_Log.AddMessageFormat("REMOVEME isNumber %s", token.c_str());
		size_t len = token.length();
		//Need to distinguish an operator token '-' operator from a negative number like "-5".
		//The token will always have a length of 1 character.
		if(len == 1)
		{
			if(token[0] == '-')
				return false;
		}
		for(size_t i = 0; i < len; i++)
		{
			const char c = token[i];
			if((c < '0' || c > '9') && (c != '.') && (c != '-'))
				return false;
		}
		return true;
	}
	static bool isLeftParenthesis(const std::string& token)
	{
		if(token.compare("(") == 0)
			return true;
		return false;
	}
	static bool isRightParenthesis(const std::string& token)
	{
		if(token.compare(")") == 0)
			return true;
		return false;
	}
	static bool isOperator(const std::string& token)
	{
		if(token.length() != 1)
			return false;

		switch(token[0])
		{
		case '+':
		case '-':
		case '*':
		case '/':
			return true;
		}
		return false;
	}
	static bool isNegativeNumber(const std::string& str, size_t pos)
	{
		//When calling this function, the character at <pos> should be '-'
		//The following character will determine whether this is negative numeric value.
		if(++pos > (str.size() - 1))
			return false;

		if(isNumericChar(str[pos]) == true && str[pos] != '-')
			return true;

		return false;
	}
	
	static bool isVariable(const std::string& test)
	{
		if(test.size() == 0)
			return false;
		for(size_t i = 0; i < test.size(); i++)
		{
			if(test[i] >= 'a' && test[i] <= 'z')
				continue;
			if(test[i] >= 'A' && test[i] <= 'Z')
				continue;
			if(test[i] == '_')
				continue;
			return false;
		}
		return true;
	}
	static double ResolveValue(const std::string& value, AbilityManager2 *symbolResolver, bool testEvaluationOnly)
	{
		if(isNumber(value) == true)
			return atof(value.c_str());
		else
		{
			if(testEvaluationOnly == true)
				return 0.0;

			if(symbolResolver != NULL)
				return symbolResolver->ResolveSymbol(value);

			return 0.0;
		}
	}
};
