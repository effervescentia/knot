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
        Assert.string("&&", OB.LogicalAnd |> ~@BinaryOperator.format_operator)
    ),
    "pp_binary_op() - logical or"
    >: (
      () =>
        Assert.string("||", OB.LogicalOr |> ~@BinaryOperator.format_operator)
    ),
    "pp_binary_op() - add"
    >: (() => Assert.string("+", OB.Add |> ~@BinaryOperator.format_operator)),
    "pp_binary_op() - subtract"
    >: (
      () =>
        Assert.string("-", OB.Subtract |> ~@BinaryOperator.format_operator)
    ),
    "pp_binary_op() - divide"
    >: (
      () => Assert.string("/", OB.Divide |> ~@BinaryOperator.format_operator)
    ),
    "pp_binary_op() - multiply"
    >: (
      () =>
        Assert.string("*", OB.Multiply |> ~@BinaryOperator.format_operator)
    ),
    "pp_binary_op() - less than or equal"
    >: (
      () =>
        Assert.string(
          "<=",
          OB.LessOrEqual |> ~@BinaryOperator.format_operator,
        )
    ),
    "pp_binary_op() - less than"
    >: (
      () =>
        Assert.string("<", OB.LessThan |> ~@BinaryOperator.format_operator)
    ),
    "pp_binary_op() - greater than or equal"
    >: (
      () =>
        Assert.string(
          ">=",
          OB.GreaterOrEqual |> ~@BinaryOperator.format_operator,
        )
    ),
    "pp_binary_op() - greater than"
    >: (
      () =>
        Assert.string(">", OB.GreaterThan |> ~@BinaryOperator.format_operator)
    ),
    "pp_binary_op() - equal"
    >: (
      () => Assert.string("==", OB.Equal |> ~@BinaryOperator.format_operator)
    ),
    "pp_binary_op() - unequal"
    >: (
      () =>
        Assert.string("!=", OB.Unequal |> ~@BinaryOperator.format_operator)
    ),
    "pp_binary_op() - exponent"
    >: (
      () =>
        Assert.string("^", OB.Exponent |> ~@BinaryOperator.format_operator)
    ),
    "pp_unary_op() - not"
    >: (() => Assert.string("!", OU.Not |> ~@UnaryOperator.format_operator)),
    "pp_unary_op() - positive"
    >: (
      () => Assert.string("+", OU.Positive |> ~@UnaryOperator.format_operator)
    ),
    "pp_unary_op() - negative"
    >: (
      () => Assert.string("-", OU.Negative |> ~@UnaryOperator.format_operator)
    ),
    "pp_ns() - internal"
    >: (
      () =>
        Assert.string(
          "\"@/fooBar\"",
          Reference.Namespace.Internal("fooBar") |> ~@Import.format_namespace,
        )
    ),
    "pp_ns() - external"
    >: (
      () =>
        Assert.string(
          "\"fooBar\"",
          Reference.Namespace.External("fooBar") |> ~@Import.format_namespace,
        )
    ),
    "pp_num() - integer"
    >: (
      () =>
        Assert.string(
          "123",
          123L |> Primitive.of_integer |> ~@Primitive.format_primitive,
        )
    ),
    "pp_num() - maximum integer"
    >: (
      () =>
        Assert.string(
          "9223372036854775807",
          Int64.max_int |> Primitive.of_integer |> ~@Primitive.format_primitive,
        )
    ),
    "pp_num() - minimum integer"
    >: (
      () =>
        Assert.string(
          "-9223372036854775808",
          Int64.min_int |> Primitive.of_integer |> ~@Primitive.format_primitive,
        )
    ),
    "pp_num() - float"
    >: (
      () =>
        Assert.string(
          "123.456",
          (123.456, 3) |> Primitive.of_float |> ~@Primitive.format_primitive,
        )
    ),
    "pp_prim() - number"
    >: (
      () =>
        Assert.string(
          "123",
          123L |> Primitive.of_integer |> ~@Primitive.format_primitive,
        )
    ),
    "pp_prim() - boolean"
    >: (
      () =>
        Assert.string(
          "true",
          true |> Primitive.of_boolean |> ~@Primitive.format_primitive,
        )
    ),
    "pp_prim() - nil"
    >: (
      () =>
        Assert.string("nil", Primitive.nil |> ~@Primitive.format_primitive)
    ),
    "pp_prim() - string"
    >: (
      () =>
        Assert.string(
          "\"foo bar\"",
          "foo bar" |> Primitive.of_string |> ~@Primitive.format_primitive,
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
