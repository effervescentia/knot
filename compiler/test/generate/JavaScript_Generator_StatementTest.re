open Kore;
open Util.ResultUtil;
open Generate.JavaScript_AST;

module Generator = Generate.JavaScript_Generator;
module Formatter = Generate.JavaScript_Formatter;

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

let __variable_declaration =
  ("fooBar" |> of_public |> as_raw_node, int_prim(123)) |> of_var;

let __expression =
  (int_prim(123), int_prim(456)) |> of_eq_op |> as_int |> of_expr;

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
          ("foo" |> of_public |> as_raw_node, int_prim(123)),
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
            "foo" |> of_public |> as_raw_node,
            [
              {
                name: "bar" |> of_public |> as_raw_node,
                default: None,
                type_: None,
              }
              |> as_nil,
            ],
            int_prim(123),
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
                Assignment(Identifier("bar"), Number("123")),
                Return(
                  Some(
                    Group(BinaryOp("+", Identifier("bar"), Number("5"))),
                  ),
                ),
              ],
            ),
          ),
          (
            "foo" |> of_public |> as_raw_node,
            [
              {
                name: "bar" |> of_public |> as_raw_node,
                default: Some(int_prim(123)),
                type_: None,
              }
              |> as_nil,
            ],
            ("bar" |> of_public |> as_int |> of_id |> as_int, int_prim(5))
            |> of_add_op
            |> as_int,
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
            "foo" |> of_public |> as_raw_node,
            [],
            [
              ("buzz" |> of_public |> as_raw_node, int_prim(2))
              |> of_var
              |> as_nil,
              (
                "buzz" |> of_public |> as_int |> of_id |> as_int,
                "buzz" |> of_public |> as_int |> of_id |> as_int,
              )
              |> of_div_op
              |> as_float
              |> of_expr
              |> as_float,
            ]
            |> of_closure
            |> as_float,
          ),
        )
    ),
  ];
