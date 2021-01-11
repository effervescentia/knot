open Infix;

type binary_operator_t =
  /* logical operators */
  | LogicalAnd
  | LogicalOr
  /* comparative operators */
  | LessOrEqual
  | LessThan
  | GreaterOrEqual
  | GreaterThan
  /* equality operators */
  | Equal
  | Unequal
  /* arithmetic operators */
  | Add
  | Subtract
  | Divide
  | Multiply
  | Exponent;

type unary_operator_t =
  | Not
  | Positive
  | Negative;

type number_t =
  | Integer(Int64.t)
  | Float(float);

type primitive_t =
  | Nil
  | Boolean(bool)
  | Number(Block.t(number_t))
  | String(string)
and jsx_t =
  | Tag(string, list(jsx_attribute_t), list(jsx_child_t))
  | Fragment(list(jsx_child_t))
and jsx_child_t =
  | Text(string)
  | Node(jsx_t)
  | InlineExpression(expression_t)
and jsx_attribute_t =
  | Class(string, option(expression_t))
  | ID(string, option(expression_t))
  | Property(string, option(expression_t))
and expression_t =
  | Primitive(primitive_t)
  | Identifier(string)
  | JSX(jsx_t)
  | Group(expression_t)
  | BinaryOp(binary_operator_t, expression_t, expression_t)
  | UnaryOp(unary_operator_t, expression_t)
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

/* tag helpers */

let of_import = ((m_id, main)) => Import(m_id, main);
let of_decl = ((name, x)) => Declaration(name, x);
let of_const = x => Constant(x);
let of_var = ((name, x)) => Variable(name, x);
let of_expr = x => Expression(x);
let of_id = x => Identifier(x);
let of_group = x => Group(x);
let of_closure = xs => Closure(xs);

let of_not_op = x => UnaryOp(Not, x);
let of_neg_op = x => UnaryOp(Negative, x);
let of_pos_op = x => UnaryOp(Positive, x);

let of_and_op = ((l, r)) => BinaryOp(LogicalAnd, l, r);
let of_or_op = ((l, r)) => BinaryOp(LogicalOr, l, r);

let of_mult_op = ((l, r)) => BinaryOp(Multiply, l, r);
let of_div_op = ((l, r)) => BinaryOp(Divide, l, r);
let of_add_op = ((l, r)) => BinaryOp(Add, l, r);
let of_sub_op = ((l, r)) => BinaryOp(Subtract, l, r);

let of_lt_op = ((l, r)) => BinaryOp(LessThan, l, r);
let of_lte_op = ((l, r)) => BinaryOp(LessOrEqual, l, r);
let of_gt_op = ((l, r)) => BinaryOp(GreaterThan, l, r);
let of_gte_op = ((l, r)) => BinaryOp(GreaterOrEqual, l, r);

let of_eq_op = ((l, r)) => BinaryOp(Equal, l, r);
let of_ineq_op = ((l, r)) => BinaryOp(Unequal, l, r);

let of_expo_op = ((l, r)) => BinaryOp(Exponent, l, r);

let of_jsx = x => JSX(x);
let of_frag = xs => Fragment(xs);
let of_tag = ((name, attrs, children)) => Tag(name, attrs, children);
let of_prop = ((name, value)) => Property(name, value);
let of_jsx_class = ((name, value)) => Class(name, value);
let of_jsx_id = ((name, value)) => ID(name, value);
let of_text = x => Text(x);
let of_node = x => Node(x);
let of_inline_expr = x => InlineExpression(x);

let of_prim = x => Primitive(x);
let of_bool = x => Boolean(x);
let of_int = x => Integer(x);
let of_float = x => Float(x);
let of_string = x => String(x);
let of_num = x => Number(x);
let nil = Nil;
