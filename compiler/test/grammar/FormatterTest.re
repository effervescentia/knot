open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Formatter = Grammar.Formatter;

let suite =
  "Grammar.Formatter"
  >::: [
    "pp_binary_op() - logical and"
    >: (() => Assert.string("&&", LogicalAnd |> ~@pp_binary_op)),
    "pp_binary_op() - logical or"
    >: (() => Assert.string("||", LogicalOr |> ~@pp_binary_op)),
    "pp_binary_op() - add"
    >: (() => Assert.string("+", Add |> ~@pp_binary_op)),
    "pp_binary_op() - subtract"
    >: (() => Assert.string("-", Subtract |> ~@pp_binary_op)),
    "pp_binary_op() - divide"
    >: (() => Assert.string("/", Divide |> ~@pp_binary_op)),
    "pp_binary_op() - multiply"
    >: (() => Assert.string("*", Multiply |> ~@pp_binary_op)),
    "pp_binary_op() - less than or equal"
    >: (() => Assert.string("<=", LessOrEqual |> ~@pp_binary_op)),
    "pp_binary_op() - less than"
    >: (() => Assert.string("<", LessThan |> ~@pp_binary_op)),
    "pp_binary_op() - greater than or equal"
    >: (() => Assert.string(">=", GreaterOrEqual |> ~@pp_binary_op)),
    "pp_binary_op() - greater than"
    >: (() => Assert.string(">", GreaterThan |> ~@pp_binary_op)),
    "pp_binary_op() - equal"
    >: (() => Assert.string("==", Equal |> ~@pp_binary_op)),
    "pp_binary_op() - unequal"
    >: (() => Assert.string("!=", Unequal |> ~@pp_binary_op)),
    "pp_binary_op() - exponent"
    >: (() => Assert.string("^", Exponent |> ~@pp_binary_op)),
    "pp_unary_op() - not" >: (() => Assert.string("!", Not |> ~@pp_unary_op)),
    "pp_unary_op() - positive"
    >: (() => Assert.string("+", Positive |> ~@pp_unary_op)),
    "pp_unary_op() - negative"
    >: (() => Assert.string("-", Negative |> ~@pp_unary_op)),
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
    >: (() => Assert.string("123", 123L |> of_int |> ~@pp_num)),
    "pp_num() - maximum integer"
    >: (
      () =>
        Assert.string(
          "9223372036854775807",
          Int64.max_int |> of_int |> ~@pp_num,
        )
    ),
    "pp_num() - minimum integer"
    >: (
      () =>
        Assert.string(
          "-9223372036854775808",
          Int64.min_int |> of_int |> ~@pp_num,
        )
    ),
    "pp_num() - float"
    >: (() => Assert.string("123.456", (123.456, 3) |> of_float |> ~@pp_num)),
    "pp_prim() - number"
    >: (() => Assert.string("123", 123L |> of_int |> of_num |> ~@pp_prim)),
    "pp_prim() - boolean"
    >: (() => Assert.string("true", true |> of_bool |> ~@pp_prim)),
    "pp_prim() - nil" >: (() => Assert.string("nil", nil |> ~@pp_prim)),
    "pp_prim() - string"
    >: (
      () => Assert.string("\"foo bar\"", "foo bar" |> of_string |> ~@pp_prim)
    ),
    "pp_statement() - expression"
    >: (
      () =>
        Assert.string(
          "nil;",
          nil |> as_nil |> of_prim |> as_nil |> of_expr |> ~@pp_statement,
        )
    ),
    "pp_statement() - variable declaration"
    >: (
      () =>
        Assert.string(
          "let foo = nil;",
          (
            "foo" |> of_public |> as_raw_node,
            nil |> as_nil |> of_prim |> as_nil,
          )
          |> of_var
          |> ~@pp_statement,
        )
    ),
  ];
