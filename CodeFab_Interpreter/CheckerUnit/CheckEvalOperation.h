#pragma once
#include <optional>

#include "./CheckerError.h"
#include "../Expr.h"

class CheckEvalOperation
{
public:
	static std::optional<ValuableValue> evaluateBinaryCalc(
		const Token& op, const ValuableValue& l_value, const ValuableValue& r_value);
	static std::optional<ValuableValue> evaluateUnaryCalc(
		const Token& op, const ValuableValue& value);

private:
	static std::optional<ValuableValue> evaluateArithmeticOp(
		const Token& op, const ValuableValue& l_value, const ValuableValue& r_value);
	static std::optional<ValuableValue> evaluateComparisonOp(
		const Token& op, const ValuableValue& l_value, const ValuableValue& r_value);
	static std::optional<ValuableValue> evaluateEqualityOp(
		const Token& op, const ValuableValue& l_value, const ValuableValue& r_value);
	static std::optional<ValuableValue> evaluateLogicalOp(
		const Token& op, const ValuableValue& l_value, const ValuableValue& r_value);
};
