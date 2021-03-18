/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
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
  | Float(float, int);

type lexeme_t('a) = ('a, Cursor.t);
type typed_lexeme_t('a) = ('a, Type.t, Cursor.t);

type primitive_t = typed_lexeme_t(_primitive_t)
and _primitive_t =
  | Nil
  | Boolean(bool)
  | Number(number_t)
  | String(string)
and jsx_t =
  | Tag(identifier_t, list(jsx_attribute_t), list(jsx_child_t))
  | Fragment(list(jsx_child_t))
and jsx_child_t =
  | Text(string)
  | Node(jsx_t)
  | InlineExpression(expression_t)
and jsx_attribute_t =
  | ID(identifier_t)
  | Class(identifier_t, option(expression_t))
  | Property(identifier_t, option(expression_t))
/* and identifier_t = (_identifier_t, Cursor.t) */
and identifier_t =
  | Private(string)
  | Public(string)
/* and expression_t = (_expression_t, Type.t, Cursor.t) */
and expression_t =
  | Primitive(primitive_t)
  | Identifier(identifier_t)
  | JSX(jsx_t)
  | Group(Block.t(expression_t))
  | BinaryOp(binary_operator_t, expression_t, expression_t)
  | UnaryOp(unary_operator_t, expression_t)
  | Closure(Block.t(list(statement_t)))
and statement_t =
  | Variable(identifier_t, expression_t)
  | Expression(expression_t);
/* | Primitive(primitive_t)
     | Identifier(identifier_t)
     | JSX(jsx_t)
     | Group(expression_t)
     | BinaryOp(binary_operator_t, expression_t, expression_t)
     | UnaryOp(unary_operator_t, expression_t)
     | Closure(list(statement_t))
   and statement_t =
     | Variable(identifier_t, expression_t)
     | Expression(expression_t)
     | EmptyStatement; */

type declaration_t =
  | Constant(expression_t);

type namespace_t =
  | Internal(string)
  | External(string);

type module_statement_t =
  | Import(namespace_t, string)
  | Declaration(identifier_t, declaration_t);

type program_t = list(module_statement_t);

/* tag helpers */

let of_internal = namespace => Internal(namespace);
let of_external = namespace => External(namespace);

let of_import = ((namespace, main)) => Import(namespace, main);
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
let of_jsx_id = name => ID(name);
let of_text = x => Text(x);
let of_node = x => Node(x);
let of_inline_expr = x => InlineExpression(x);

let of_public = x => Public(x);
let of_private = x => Private(x);

let of_prim = x => Primitive(x);
let of_bool = x => Boolean(x);
let of_int = x => Integer(x);
let of_float = ((x, precision)) => Float(x, precision);
let of_string = x => String(x);
let of_num = x => Number(x);
let nil = Nil;

/* printing utils */

let string_of_namespace =
  fun
  | Internal(path) => Constants.root_dir ++ path
  | External(path) => path;
