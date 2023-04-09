open Kore;

module OB = AST.Operator.Binary;
module OU = AST.Operator.Unary;
module U = Util.ResultUtil;

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
      () =>
        Assert.string(
          "123",
          123L
          |> Primitive.of_integer
          |> ~@Primitive.Formatter.format_primitive,
        )
    ),
    "pp_num() - maximum integer"
    >: (
      () =>
        Assert.string(
          "9223372036854775807",
          Int64.max_int
          |> Primitive.of_integer
          |> ~@Primitive.Formatter.format_primitive,
        )
    ),
    "pp_num() - minimum integer"
    >: (
      () =>
        Assert.string(
          "-9223372036854775808",
          Int64.min_int
          |> Primitive.of_integer
          |> ~@Primitive.Formatter.format_primitive,
        )
    ),
    "pp_num() - float"
    >: (
      () =>
        Assert.string(
          "123.456",
          (123.456, 3)
          |> Primitive.of_float
          |> ~@Primitive.Formatter.format_primitive,
        )
    ),
    "pp_prim() - number"
    >: (
      () =>
        Assert.string(
          "123",
          123L
          |> Primitive.of_integer
          |> ~@Primitive.Formatter.format_primitive,
        )
    ),
    "pp_prim() - boolean"
    >: (
      () =>
        Assert.string(
          "true",
          true
          |> Primitive.of_boolean
          |> ~@Primitive.Formatter.format_primitive,
        )
    ),
    "pp_prim() - nil"
    >: (
      () =>
        Assert.string(
          "nil",
          Primitive.nil |> ~@Primitive.Formatter.format_primitive,
        )
    ),
    "pp_prim() - string"
    >: (
      () =>
        Assert.string(
          "\"foo bar\"",
          "foo bar"
          |> Primitive.of_string
          |> ~@Primitive.Formatter.format_primitive,
        )
    ),
    "pp_statement() - expression"
    >: (
      () =>
        Assert.string(
          "nil;",
          U.nil_prim
          |> Statement.of_effect
          |> ~@Statement.format((), Expression.format),
        )
    ),
    "pp_statement() - variable declaration"
    >: (
      () =>
        Assert.string(
          "let foo = nil;",
          (U.as_untyped("foo"), U.nil_prim)
          |> Statement.of_variable
          |> ~@Statement.format((), Expression.format),
        )
    ),
  ];
