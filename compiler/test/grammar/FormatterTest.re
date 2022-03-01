open Kore;

module Formatter = Grammar.Formatter;
module U = Util.ResultUtil;

let suite =
  "Grammar.Formatter"
  >::: [
    "pp_binary_op() - logical and"
    >: (() => Assert.string("&&", A.LogicalAnd |> ~@pp_binary_op)),
    "pp_binary_op() - logical or"
    >: (() => Assert.string("||", A.LogicalOr |> ~@pp_binary_op)),
    "pp_binary_op() - add"
    >: (() => Assert.string("+", A.Add |> ~@pp_binary_op)),
    "pp_binary_op() - subtract"
    >: (() => Assert.string("-", A.Subtract |> ~@pp_binary_op)),
    "pp_binary_op() - divide"
    >: (() => Assert.string("/", A.Divide |> ~@pp_binary_op)),
    "pp_binary_op() - multiply"
    >: (() => Assert.string("*", A.Multiply |> ~@pp_binary_op)),
    "pp_binary_op() - less than or equal"
    >: (() => Assert.string("<=", A.LessOrEqual |> ~@pp_binary_op)),
    "pp_binary_op() - less than"
    >: (() => Assert.string("<", A.LessThan |> ~@pp_binary_op)),
    "pp_binary_op() - greater than or equal"
    >: (() => Assert.string(">=", A.GreaterOrEqual |> ~@pp_binary_op)),
    "pp_binary_op() - greater than"
    >: (() => Assert.string(">", A.GreaterThan |> ~@pp_binary_op)),
    "pp_binary_op() - equal"
    >: (() => Assert.string("==", A.Equal |> ~@pp_binary_op)),
    "pp_binary_op() - unequal"
    >: (() => Assert.string("!=", A.Unequal |> ~@pp_binary_op)),
    "pp_binary_op() - exponent"
    >: (() => Assert.string("^", A.Exponent |> ~@pp_binary_op)),
    "pp_unary_op() - not"
    >: (() => Assert.string("!", A.Not |> ~@pp_unary_op)),
    "pp_unary_op() - positive"
    >: (() => Assert.string("+", A.Positive |> ~@pp_unary_op)),
    "pp_unary_op() - negative"
    >: (() => Assert.string("-", A.Negative |> ~@pp_unary_op)),
    "pp_ns() - internal"
    >: (
      () =>
        Assert.string(
          "\"@/fooBar\"",
          Namespace.Internal("fooBar") |> ~@pp_ns,
        )
    ),
    "pp_ns() - external"
    >: (
      () =>
        Assert.string("\"fooBar\"", Namespace.External("fooBar") |> ~@pp_ns)
    ),
    "pp_num() - integer"
    >: (() => Assert.string("123", 123L |> A.of_int |> ~@pp_num)),
    "pp_num() - maximum integer"
    >: (
      () =>
        Assert.string(
          "9223372036854775807",
          Int64.max_int |> A.of_int |> ~@pp_num,
        )
    ),
    "pp_num() - minimum integer"
    >: (
      () =>
        Assert.string(
          "-9223372036854775808",
          Int64.min_int |> A.of_int |> ~@pp_num,
        )
    ),
    "pp_num() - float"
    >: (
      () => Assert.string("123.456", (123.456, 3) |> A.of_float |> ~@pp_num)
    ),
    "pp_prim() - number"
    >: (() => Assert.string("123", 123L |> A.of_int |> A.of_num |> ~@pp_prim)),
    "pp_prim() - boolean"
    >: (() => Assert.string("true", true |> A.of_bool |> ~@pp_prim)),
    "pp_prim() - nil" >: (() => Assert.string("nil", A.nil |> ~@pp_prim)),
    "pp_prim() - string"
    >: (
      () =>
        Assert.string("\"foo bar\"", "foo bar" |> A.of_string |> ~@pp_prim)
    ),
    "pp_statement() - expression"
    >: (
      () => Assert.string("nil;", U.nil_prim |> A.of_expr |> ~@pp_statement)
    ),
    "pp_statement() - variable declaration"
    >: (
      () =>
        Assert.string(
          "let foo = nil;",
          ("foo" |> A.of_public |> U.as_raw_node, U.nil_prim)
          |> A.of_var
          |> ~@pp_statement,
        )
    ),
  ];
