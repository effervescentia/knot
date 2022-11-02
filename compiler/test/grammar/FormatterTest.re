open Kore;

module A = AST.Result;
module Formatter = Language.Formatter;
module U = Util.ResultUtil;

let suite =
  "Grammar.Formatter"
  >::: [
    "pp_binary_op() - logical and"
    >: (
      () =>
        Assert.string(
          "&&",
          A.LogicalAnd |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - logical or"
    >: (
      () =>
        Assert.string(
          "||",
          A.LogicalOr |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - add"
    >: (
      () =>
        Assert.string("+", A.Add |> ~@KBinaryOperator.Formatter.pp_operator)
    ),
    "pp_binary_op() - subtract"
    >: (
      () =>
        Assert.string(
          "-",
          A.Subtract |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - divide"
    >: (
      () =>
        Assert.string(
          "/",
          A.Divide |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - multiply"
    >: (
      () =>
        Assert.string(
          "*",
          A.Multiply |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - less than or equal"
    >: (
      () =>
        Assert.string(
          "<=",
          A.LessOrEqual |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - less than"
    >: (
      () =>
        Assert.string(
          "<",
          A.LessThan |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - greater than or equal"
    >: (
      () =>
        Assert.string(
          ">=",
          A.GreaterOrEqual |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - greater than"
    >: (
      () =>
        Assert.string(
          ">",
          A.GreaterThan |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - equal"
    >: (
      () =>
        Assert.string(
          "==",
          A.Equal |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - unequal"
    >: (
      () =>
        Assert.string(
          "!=",
          A.Unequal |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_binary_op() - exponent"
    >: (
      () =>
        Assert.string(
          "^",
          A.Exponent |> ~@KBinaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_unary_op() - not"
    >: (
      () =>
        Assert.string("!", A.Not |> ~@KUnaryOperator.Formatter.pp_operator)
    ),
    "pp_unary_op() - positive"
    >: (
      () =>
        Assert.string(
          "+",
          A.Positive |> ~@KUnaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_unary_op() - negative"
    >: (
      () =>
        Assert.string(
          "-",
          A.Negative |> ~@KUnaryOperator.Formatter.pp_operator,
        )
    ),
    "pp_ns() - internal"
    >: (
      () =>
        Assert.string(
          "\"@/fooBar\"",
          Reference.Namespace.Internal("fooBar")
          |> ~@KImport.Formatter.pp_namespace,
        )
    ),
    "pp_ns() - external"
    >: (
      () =>
        Assert.string(
          "\"fooBar\"",
          Reference.Namespace.External("fooBar")
          |> ~@KImport.Formatter.pp_namespace,
        )
    ),
    "pp_num() - integer"
    >: (() => Assert.string("123", 123L |> A.of_int |> ~@KNumber.Plugin.pp)),
    "pp_num() - maximum integer"
    >: (
      () =>
        Assert.string(
          "9223372036854775807",
          Int64.max_int |> A.of_int |> ~@KNumber.Plugin.pp,
        )
    ),
    "pp_num() - minimum integer"
    >: (
      () =>
        Assert.string(
          "-9223372036854775808",
          Int64.min_int |> A.of_int |> ~@KNumber.Plugin.pp,
        )
    ),
    "pp_num() - float"
    >: (
      () =>
        Assert.string(
          "123.456",
          (123.456, 3) |> A.of_float |> ~@KNumber.Plugin.pp,
        )
    ),
    "pp_prim() - number"
    >: (
      () =>
        Assert.string(
          "123",
          123L |> A.of_int |> A.of_num |> ~@KPrimitive.Plugin.pp,
        )
    ),
    "pp_prim() - boolean"
    >: (
      () => Assert.string("true", true |> A.of_bool |> ~@KPrimitive.Plugin.pp)
    ),
    "pp_prim() - nil"
    >: (() => Assert.string("nil", A.nil |> ~@KPrimitive.Plugin.pp)),
    "pp_prim() - string"
    >: (
      () =>
        Assert.string(
          "\"foo bar\"",
          "foo bar" |> A.of_string |> ~@KPrimitive.Plugin.pp,
        )
    ),
    "pp_statement() - expression"
    >: (
      () =>
        Assert.string(
          "nil;",
          U.nil_prim
          |> A.of_expr
          |> ~@KStatement.Plugin.pp(KExpression.Plugin.pp),
        )
    ),
    "pp_statement() - variable declaration"
    >: (
      () =>
        Assert.string(
          "let foo = nil;",
          (U.as_untyped("foo"), U.nil_prim)
          |> A.of_var
          |> ~@KStatement.Plugin.pp(KExpression.Plugin.pp),
        )
    ),
  ];
