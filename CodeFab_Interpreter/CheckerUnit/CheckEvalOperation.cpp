#include "CheckEvalOperation.h"

static auto isNum  = [](const ValuableValue& v) { return std::holds_alternative<double>(v); };
static auto isStr  = [](const ValuableValue& v) { return std::holds_alternative<std::string>(v); };
static auto isBool = [](const ValuableValue& v) { return std::holds_alternative<bool>(v); };
static auto num    = [](const ValuableValue& v) { return std::get<double>(v); };
static auto str    = [](const ValuableValue& v) -> const std::string& { return std::get<std::string>(v); };

static std::optional<ValuableValue> throwTypeMismatch(const Token& op, const std::string& msg) {
	throw CheckerError("[line " + std::to_string(op.getLine()) + "] Checker Error: " + msg);
}

std::optional<ValuableValue> CheckEvalOperation::evaluateBinaryCalc(
	const Token& op, const ValuableValue& l_value, const ValuableValue& r_value)
{
	switch (op.getTokenType()) {
	case TokenType::PLUS:
	case TokenType::MINUS:
	case TokenType::STAR:
	case TokenType::SLASH:
		return evaluateArithmeticOp(op, l_value, r_value);

	case TokenType::LESS:
	case TokenType::LESS_EQUAL:
	case TokenType::GREATER:
	case TokenType::GREATER_EQUAL:
		return evaluateComparisonOp(op, l_value, r_value);

	case TokenType::EQUAL_EQUAL:
	case TokenType::BANG_EQUAL:
		return evaluateEqualityOp(op, l_value, r_value);

	case TokenType::AND_OP:
	case TokenType::OR_OP:
		return evaluateLogicalOp(op, l_value, r_value);

	default:
		throw CheckerError("[line " + std::to_string(op.getLine()) +
			"] Checker Error: Unknown binary operator.");
	}
}

std::optional<ValuableValue> CheckEvalOperation::evaluateUnaryCalc(
	const Token& op, const ValuableValue& v)
{
	if (op.getTokenType() == TokenType::MINUS) {
		if (!isNum(v))
			throw CheckerError("[line " + std::to_string(op.getLine()) +
				"] Checker Error: Operand must be a number.");
		return -std::get<double>(v);
	}
	if (op.getTokenType() == TokenType::BANG) {
		if (std::holds_alternative<std::nullptr_t>(v)) return ValuableValue(true);
		if (isBool(v)) return ValuableValue(!std::get<bool>(v));
		throw CheckerError("[line " + std::to_string(op.getLine()) +
			"] Checker Error: Operand must be a boolean or nullptr.");
	}
	throw CheckerError("[line " + std::to_string(op.getLine()) +
		"] Checker Error: Unknown unary operator.");
}

std::optional<ValuableValue> CheckEvalOperation::evaluateArithmeticOp(
	const Token& op, const ValuableValue& l_value, const ValuableValue& r_value)
{
	switch (op.getTokenType()) {
	case TokenType::PLUS:
		if (isNum(l_value) && isNum(r_value)) return num(l_value) + num(r_value);
		if (isStr(l_value) && isStr(r_value)) return str(l_value) + str(r_value);
		return throwTypeMismatch(op, "Operands must be two numbers or two strings.");
	case TokenType::MINUS:
		if (!isNum(l_value) || !isNum(r_value)) return throwTypeMismatch(op, "Operands must be numbers.");
		return num(l_value) - num(r_value);
	case TokenType::STAR:
		if (!isNum(l_value) || !isNum(r_value)) return throwTypeMismatch(op, "Operands must be numbers.");
		return num(l_value) * num(r_value);
	case TokenType::SLASH:
		if (!isNum(l_value) || !isNum(r_value)) return throwTypeMismatch(op, "Operands must be numbers.");
		if (num(r_value) == 0.0)
			throw CheckerError("[line " + std::to_string(op.getLine()) +
				"] Checker Error: Division by zero.");
		return num(l_value) / num(r_value);
	default:
		return {};
	}
}

std::optional<ValuableValue> CheckEvalOperation::evaluateComparisonOp(
	const Token& op, const ValuableValue& l_value, const ValuableValue& r_value)
{
	if (!isNum(l_value) || !isNum(r_value))
		return throwTypeMismatch(op, "Operands must be numbers.");

	switch (op.getTokenType()) {
	case TokenType::LESS:          return num(l_value) <  num(r_value);
	case TokenType::LESS_EQUAL:    return num(l_value) <= num(r_value);
	case TokenType::GREATER:       return num(l_value) >  num(r_value);
	case TokenType::GREATER_EQUAL: return num(l_value) >= num(r_value);
	default:                       return {};
	}
}

std::optional<ValuableValue> CheckEvalOperation::evaluateEqualityOp(
	const Token& op, const ValuableValue& l_value, const ValuableValue& r_value)
{
	if (l_value.index() != r_value.index())
		return throwTypeMismatch(op, "Operands must be the same type.");

	bool eq;
	if (std::holds_alternative<std::nullptr_t>(l_value))  eq = true;
	else if (isBool(l_value))  eq = std::get<bool>(l_value) == std::get<bool>(r_value);
	else if (isNum(l_value))   eq = num(l_value) == num(r_value);
	else if (isStr(l_value))   eq = str(l_value) == str(r_value);
	else return {};

	return ValuableValue(op.getTokenType() == TokenType::EQUAL_EQUAL ? eq : !eq);
}

std::optional<ValuableValue> CheckEvalOperation::evaluateLogicalOp(
	const Token& op, const ValuableValue& l_value, const ValuableValue& r_value)
{
	if (!isBool(l_value) || !isBool(r_value))
		return throwTypeMismatch(op, "Operands must be booleans.");

	bool l = std::get<bool>(l_value);
	bool r = std::get<bool>(r_value);
	return ValuableValue(op.getTokenType() == TokenType::AND_OP ? l && r : l || r);
}
