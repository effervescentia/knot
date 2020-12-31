type number_t =
  | Integer(int)
  | Float(float);

type primitive_t =
  | Boolean(bool)
  | Number(number_t);

type binary_operator_t =
  | Add
  | Subtract
  | Divide
  | Multiply;

type unary_operator_t =
  | Not
  | Negative;

type expression_t =
  | Primitive(primitive_t)
  | Identifier(string)
  | Group(expression_t)
  | BinaryOperation(binary_operator_t, expression_t, expression_t)
  | UnaryOperation(unary_operator_t, expression_t)
  | Closure(list(statement_t))
and statement_t =
  | Variable(string, expression_t)
  | Expression(expression_t)
  | EmptyStatement;

type declaration_t =
  | Constant(expression_t);

type module_statement_t =
  | Import(string, string)
  | Declaration(string, declaration_t)
  | EmptyModuleStatement;

type program_t = list(module_statement_t);
