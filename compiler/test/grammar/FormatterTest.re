open Kore;

module OB = AST.Operator.Binary;
module OU = AST.Operator.Unary;

let suite =
  "Grammar.Formatter"
  >::: [
    "pp_binary_op() - logical and"
    >: (
      () =>
        Assert.string(
          "&&",
          OB.LogicalAnd |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - logical or"
    >: (
      () =>
        Assert.string(
          "||",
          OB.LogicalOr |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - add"
    >: (
      () =>
        Assert.string(
          "+",
          OB.Add |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - subtract"
    >: (
      () =>
        Assert.string(
          "-",
          OB.Subtract |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - divide"
    >: (
      () =>
        Assert.string(
          "/",
          OB.Divide |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - multiply"
    >: (
      () =>
        Assert.string(
          "*",
          OB.Multiply |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - less than or equal"
    >: (
      () =>
        Assert.string(
          "<=",
          OB.LessOrEqual |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - less than"
    >: (
      () =>
        Assert.string(
          "<",
          OB.LessThan |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - greater than or equal"
    >: (
      () =>
        Assert.string(
          ">=",
          OB.GreaterOrEqual |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - greater than"
    >: (
      () =>
        Assert.string(
          ">",
          OB.GreaterThan |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - equal"
    >: (
      () =>
        Assert.string(
          "==",
          OB.Equal |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - unequal"
    >: (
      () =>
        Assert.string(
          "!=",
          OB.Unequal |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_binary_op() - exponent"
    >: (
      () =>
        Assert.string(
          "^",
          OB.Exponent |> ~@KBinaryOperator.Formatter.format_operator,
        )
    ),
    "pp_unary_op() - not"
    >: (
      () =>
        Assert.string(
          "!",
          OU.Not |> ~@KUnaryOperator.Formatter.format_operator,
        )
    ),
    "pp_unary_op() - positive"
    >: (
      () =>
        Assert.string(
          "+",
          OU.Positive |> ~@KUnaryOperator.Formatter.format_operator,
        )
    ),
    "pp_unary_op() - negative"
    >: (
      () =>
        Assert.string(
          "-",
          OU.Negative |> ~@KUnaryOperator.Formatter.format_operator,
        )
    ),
    "pp_ns() - internal"
    >: (
      () =>
        Assert.string(
          "\"@/fooBar\"",
          Reference.Namespace.Internal("fooBar")
          |> ~@KImport.Formatter.format_namespace,
        )
    ),
    "pp_ns() - external"
    >: (
      () =>
        Assert.string(
          "\"fooBar\"",
          Reference.Namespace.External("fooBar")
          |> ~@KImport.Formatter.format_namespace,
        )
    ),
    "pp_num() - integer"
    >: (
      () => Assert.string("123", 123L |> A.of_int |> ~@KNumber.Plugin.format)
    ),
    "pp_num() - maximum integer"
    >: (
      () =>
        Assert.string(
          "9223372036854775807",
          Int64.max_int |> A.of_int |> ~@KNumber.Plugin.format,
        )
    ),
    "pp_num() - minimum integer"
    >: (
      () =>
        Assert.string(
          "-9223372036854775808",
          Int64.min_int |> A.of_int |> ~@KNumber.Plugin.format,
        )
    ),
    "pp_num() - float"
    >: (
      () =>
        Assert.string(
          "123.456",
          (123.456, 3) |> A.of_float |> ~@KNumber.Plugin.format,
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
          |> ~@KStatement.Plugin.format(KExpression.Plugin.format),
        )
    ),
    "pp_statement() - variable declaration"
    >: (
      () =>
        Assert.string(
          "let foo = nil;",
          (U.as_untyped("foo"), U.nil_prim)
          |> A.of_var
          |> ~@KStatement.Plugin.format(KExpression.Plugin.format),
        )
    ),
  ];
