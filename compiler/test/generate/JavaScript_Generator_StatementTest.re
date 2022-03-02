open Kore;
open Generate.JavaScript_AST;

module Generator = Generate.JavaScript_Generator;
module Formatter = Generate.JavaScript_Formatter;
module U = Util.ResultUtil;

let _assert_statement_list =
  Alcotest.(
    check(
      list(Assert.Compare.statement(Target.Common)),
      "javascript statement list matches",
    )
  );
let _assert_constant = (expected, actual) =>
  Alcotest.(
    check(
      Assert.Compare.statement(Target.Common),
      "javascript constant matches",
      expected,
      actual |> Tuple.join2(Generator.gen_constant),
    )
  );
let _assert_function = (expected, actual) =>
  Alcotest.(
    check(
      Assert.Compare.statement(Target.Common),
      "javascript function matches",
      expected,
      actual |> Tuple.join3(Generator.gen_function),
    )
  );
let _assert_view = (expected, actual) =>
  Alcotest.(
    check(
      Assert.Compare.statement(Target.Common),
      "javascript view matches",
      expected,
      actual |> Tuple.join3(Generator.gen_view),
    )
  );

let __variable_declaration =
  ("fooBar" |> A.of_public |> U.as_raw_node, U.int_prim(123)) |> A.of_var;

let __expression =
  (U.int_prim(123), U.int_prim(456)) |> A.of_eq_op |> U.as_int |> A.of_expr;

let suite =
  "Generate.JavaScript_Generator | Statement"
  >::: [
    "variable declaration - not last in closure"
    >: (
      () =>
        _assert_statement_list(
          [Variable("fooBar", Number("123"))],
          Generator.gen_statement(__variable_declaration),
        )
    ),
    "variable declaration - last in closure"
    >: (
      () =>
        _assert_statement_list(
          [Variable("fooBar", Number("123")), Return(Some(Null))],
          Generator.gen_statement(~is_last=true, __variable_declaration),
        )
    ),
    "expression - not last in closure"
    >: (
      () =>
        _assert_statement_list(
          [
            Expression(
              Group(BinaryOp("===", Number("123"), Number("456"))),
            ),
          ],
          Generator.gen_statement(__expression),
        )
    ),
    "expression - last in closure"
    >: (
      () =>
        _assert_statement_list(
          [
            Return(
              Some(Group(BinaryOp("===", Number("123"), Number("456")))),
            ),
          ],
          Generator.gen_statement(~is_last=true, __expression),
        )
    ),
    "constant"
    >: (
      () =>
        _assert_constant(
          Variable("foo", Number("123")),
          ("foo" |> A.of_public |> U.as_raw_node, U.int_prim(123)),
        )
    ),
    "function - return primitive value"
    >: (
      () =>
        _assert_function(
          Expression(
            Function(
              Some("foo"),
              ["bar"],
              [Return(Some(Number("123")))],
            ),
          ),
          (
            "foo" |> A.of_public |> U.as_raw_node,
            [
              A.{
                name: "bar" |> A.of_public |> U.as_raw_node,
                default: None,
                type_: None,
              }
              |> U.as_nil,
            ],
            U.int_prim(123),
          ),
        )
    ),
    "function - argument with default value"
    >: (
      () =>
        _assert_function(
          Expression(
            Function(
              Some("foo"),
              ["bar"],
              [
                Assignment(
                  Identifier("bar"),
                  FunctionCall(
                    DotAccess(
                      DotAccess(Identifier("$knot"), "platform"),
                      "arg",
                    ),
                    [Identifier("arguments"), Number("0"), Number("123")],
                  ),
                ),
                Return(
                  Some(
                    Group(BinaryOp("+", Identifier("bar"), Number("5"))),
                  ),
                ),
              ],
            ),
          ),
          (
            "foo" |> A.of_public |> U.as_raw_node,
            [
              A.{
                name: "bar" |> A.of_public |> U.as_raw_node,
                default: Some(U.int_prim(123)),
                type_: None,
              }
              |> U.as_nil,
            ],
            ("bar" |> A.of_public |> A.of_id |> U.as_int, U.int_prim(5))
            |> A.of_add_op
            |> U.as_int,
          ),
        )
    ),
    "function - return complex expression"
    >: (
      () =>
        _assert_function(
          Expression(
            Function(
              Some("foo"),
              [],
              [
                Variable("buzz", Number("2")),
                Return(
                  Some(
                    Group(
                      BinaryOp("/", Identifier("buzz"), Identifier("buzz")),
                    ),
                  ),
                ),
              ],
            ),
          ),
          (
            "foo" |> A.of_public |> U.as_raw_node,
            [],
            [
              ("buzz" |> A.of_public |> U.as_raw_node, U.int_prim(2))
              |> A.of_var
              |> U.as_nil,
              (
                "buzz" |> A.of_public |> A.of_id |> U.as_int,
                "buzz" |> A.of_public |> A.of_id |> U.as_int,
              )
              |> A.of_div_op
              |> U.as_float
              |> A.of_expr
              |> U.as_float,
            ]
            |> A.of_closure
            |> U.as_float,
          ),
        )
    ),
    "view - property with default value"
    >: (
      () =>
        _assert_view(
          Expression(
            Function(
              Some("foo"),
              ["$props$"],
              [
                Assignment(
                  Identifier("bar"),
                  FunctionCall(
                    DotAccess(
                      DotAccess(Identifier("$knot"), "platform"),
                      "prop",
                    ),
                    [Identifier("$props$"), String("bar"), Number("123")],
                  ),
                ),
                Return(
                  Some(
                    Group(BinaryOp("+", Identifier("bar"), Number("5"))),
                  ),
                ),
              ],
            ),
          ),
          (
            "foo" |> A.of_public |> U.as_raw_node,
            [
              A.{
                name: "bar" |> A.of_public |> U.as_raw_node,
                default: Some(U.int_prim(123)),
                type_: None,
              }
              |> U.as_nil,
            ],
            ("bar" |> A.of_public |> A.of_id |> U.as_int, U.int_prim(5))
            |> A.of_add_op
            |> U.as_int,
          ),
        )
    ),
  ];
