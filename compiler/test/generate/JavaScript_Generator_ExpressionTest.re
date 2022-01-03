open Kore;
open Util.ResultUtil;
open Generate.JavaScript_AST;

module Generator = Generate.JavaScript_Generator;
module Formatter = Generate.JavaScript_Formatter;

let _assert_expression = (expected, actual) =>
  Alcotest.(
    check(
      Assert.Compare.expression(Target.Common),
      "javascript expression matches",
      expected,
      Generator.gen_expression(actual),
    )
  );
let _assert_binary_op = (expected, actual) =>
  Alcotest.(
    check(
      Assert.Compare.expression(Target.Common),
      "javascript binary operation matches",
      expected,
      actual |> Tuple.join3(Generator.gen_binary_op),
    )
  );
let _assert_unary_op = (expected, actual) =>
  Alcotest.(
    check(
      Assert.Compare.expression(Target.Common),
      "javascript unary operation matches",
      expected,
      actual |> Tuple.join2(Generator.gen_unary_op),
    )
  );

let suite =
  "Generate.JavaScript_Generator | Expression"
  >::: [
    "boolean - true"
    >: (
      () =>
        _assert_expression(
          Boolean(true),
          true |> of_bool |> as_bool |> of_prim,
        )
    ),
    "boolean - false"
    >: (
      () =>
        _assert_expression(
          Boolean(false),
          false |> of_bool |> as_bool |> of_prim,
        )
    ),
    "string - no special characters"
    >: (
      () =>
        _assert_expression(
          String("hello world"),
          "hello world" |> of_string |> as_string |> of_prim,
        )
    ),
    "string - escaped quotation marks"
    >: (
      () =>
        _assert_expression(
          String("escaped quotes (\")"),
          "escaped quotes (\")" |> of_string |> as_string |> of_prim,
        )
    ),
    "null" >: (() => _assert_expression(Null, nil |> as_nil |> of_prim)),
    "identifier"
    >: (
      () =>
        _assert_expression(
          Identifier("fooBar"),
          "fooBar" |> of_public |> as_generic(0, 0) |> of_id,
        )
    ),
    "group"
    >: (
      () =>
        _assert_expression(
          Group(Number("123")),
          123 |> int_prim |> of_group,
        )
    ),
    "iife - iife with return value"
    >: (
      () =>
        _assert_expression(
          FunctionCall(
            Group(
              Function(
                None,
                [],
                [
                  Expression(
                    Group(BinaryOp("===", Number("123"), Number("456"))),
                  ),
                  Return(
                    Some(
                      Group(BinaryOp("+", Number("678"), Number("910"))),
                    ),
                  ),
                ],
              ),
            ),
            [],
          ),
          [
            (123 |> int_prim, 456 |> int_prim)
            |> of_eq_op
            |> as_int
            |> of_expr
            |> as_int,
            (678 |> int_prim, 910 |> int_prim)
            |> of_add_op
            |> as_int
            |> of_expr
            |> as_int,
          ]
          |> of_closure,
        )
    ),
    "iife - variable declaration"
    >: (
      () =>
        _assert_expression(
          FunctionCall(
            Group(
              Function(
                None,
                [],
                [Variable("foo", Number("456")), Return(Some(Null))],
              ),
            ),
            [],
          ),
          [
            ("foo" |> of_public |> as_raw_node, 456 |> int_prim)
            |> of_var
            |> as_nil,
          ]
          |> of_closure,
        )
    ),
    "binary operation - logical and"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("&&", Boolean(true), Boolean(false))),
          (LogicalAnd, true |> bool_prim, false |> bool_prim),
        )
    ),
    "binary operation - logical or"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("||", Boolean(true), Boolean(false))),
          (LogicalOr, true |> bool_prim, false |> bool_prim),
        )
    ),
    "binary operation - less than or equal"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("<=", Number("123"), Number("456"))),
          (LessOrEqual, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - less than"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("<", Number("123"), Number("456"))),
          (LessThan, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - greater than or equal"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp(">=", Number("123"), Number("456"))),
          (GreaterOrEqual, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - greater than"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp(">", Number("123"), Number("456"))),
          (GreaterThan, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - equal"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("===", Number("123"), Number("456"))),
          (Equal, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - unequal"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("!==", Number("123"), Number("456"))),
          (Unequal, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - add"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("+", Number("123"), Number("456"))),
          (Add, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - subtract"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("-", Number("123"), Number("456"))),
          (Subtract, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - multiply"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("*", Number("123"), Number("456"))),
          (Multiply, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - divide"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("/", Number("123"), Number("456"))),
          (Divide, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "binary operation - exponent"
    >: (
      () =>
        _assert_binary_op(
          FunctionCall(
            DotAccess(Identifier("Math"), "pow"),
            [Number("123"), Number("456")],
          ),
          (Exponent, 123 |> int_prim, 456 |> int_prim),
        )
    ),
    "unary operation - logical negation"
    >: (
      () =>
        _assert_unary_op(
          UnaryOp("!", Group(Boolean(true))),
          (Not, true |> bool_prim),
        )
    ),
    "unary operation - positive"
    >: (
      () =>
        _assert_unary_op(
          UnaryOp("+", Group(Number("123"))),
          (Positive, 123 |> int_prim),
        )
    ),
    "unary operation - negative"
    >: (
      () =>
        _assert_unary_op(
          UnaryOp("-", Group(Number("123"))),
          (Negative, 123 |> int_prim),
        )
    ),
  ];
