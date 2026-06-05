#include "Callable.h"
#include "Interpreter.h"

ValuableValue FunctionObject::call(Interpreter& interpreter, std::vector<ValuableValue>& arguments,
                                   const Token& /*paren*/) {
    // 호출마다 closure를 부모로 하는 새 환경을 만들고 파라미터를 바인딩한다.
    auto env = std::make_shared<Environment>(closure);
    const auto& params_vector = declaration.getParams();
    for (std::size_t i = 0; i < params_vector.size(); ++i)
        env->define(params_vector[i].getLexme(), arguments[i]);

    try {
        interpreter.executeBlock(declaration.getBody(), env);
    }
    catch (const ReturnException& returned) {
        return returned.value;
    }
    return nullptr; // return 문이 없으면 nil
}

ValuableValue NativeArray::call(Interpreter& /*interpreter*/, std::vector<ValuableValue>& arguments,
                                const Token& paren) {
    if (!std::holds_alternative<double>(arguments[0]))
        throw RuntimeError(paren, "Array size must be a number.");
    double size = std::get<double>(arguments[0]);
    if (size < 0)
        throw RuntimeError(paren, "Array size must not be negative.");
    return std::make_shared<ArrayValue>(static_cast<std::size_t>(size));
}
