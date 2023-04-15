open Kore;
open Parse.Kore;
open Interface;

/*
  each expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

let parse_unary =
  UnaryOperator.parse(((op, expression)) => of_unary_op(op, expression));

/* &&, || */
let rec parse_expression_0 = ctx =>
  parse_expression_1(ctx) |> BinaryOperator.parse_logical(of_binary_op)

/* ==, != */
and parse_expression_1 = ctx =>
  parse_expression_2(ctx) |> BinaryOperator.parse_comparison(of_binary_op)

/* <=, <, >=, > */
and parse_expression_2 = ctx =>
  parse_expression_3(ctx) |> BinaryOperator.parse_relational(of_binary_op)

/* ^, *, /, +, - */
and parse_expression_3 = ctx =>
  parse_expression_4(ctx) |> BinaryOperator.parse_arithmetic(of_binary_op)

/* !, +, - */
and parse_expression_4 = ctx => parse_expression_5(ctx) |> parse_unary

/* foo(bar) */
and parse_expression_5 = (ctx): AST.Framework.Interface.parse_t('expr) =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input =>
    (
      (parse_expression_6(ctx), parse_expression_0(ctx))
      |> FunctionCall.parse(of_function_call)
    )(
      input,
    )

/* foo::bar */
and parse_expression_6 = ctx =>
  (ctx, (parse_expression_7, parse_style_literal))
  |> BindStyle.parse(((kind, lhs, rhs)) => of_bind_style(kind, (lhs, rhs)))

/* foo.bar */
and parse_expression_7 = ctx =>
  parse_expression_8(ctx) |> DotAccess.parse(of_dot_access)

/* {}, () */
and parse_expression_8 = (ctx): AST.Framework.Interface.parse_t('expr) =>
  /* do not attempt to simplify this `input` argument away or expression parsing will loop forever */
  input =>
    choice(
      [
        (ctx, parse_expression_0) |> Closure.parse(of_closure),
        parse_expression_0(ctx) |> Group.parse(of_group),
        parse_term(ctx),
      ],
      input,
    )

/* skip dot access to avoid conflict with class attribute syntax */
and parse_ksx_term = ctx =>
  (parse_expression_8(ctx), parse_expression_0(ctx))
  |> FunctionCall.parse(of_function_call)
  |> parse_unary

/* { color: $red } */
and parse_style_literal = ctx =>
  (ctx, parse_expression_0) |> KStyle.Parser.parse_style_literal(of_style)

/* 2, foo, <bar /> */
and parse_term = ctx =>
  choice([
    () |> Primitive.parse(of_primitive),
    (ctx, parse_expression_0) |> Style.parse(of_style),
    ctx |> Identifier.parse(of_identifier),
    (ctx, (parse_ksx_term, parse_expression_0, parse_style_literal))
    |> KSX.parse(of_ksx),
  ]);

let parse = parse_expression_0;
