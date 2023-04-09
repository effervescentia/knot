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
let _assert_enum = (expected, actual) =>
  Alcotest.(
    check(
      Assert.Compare.statement(Target.Common),
      "javascript enum matches",
      expected,
      actual |> Tuple.join2(Generator.gen_enumerated),
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
      actual |> Tuple.join4(Generator.gen_view),
    )
  );

let __variable_declaration =
  (U.as_untyped("fooBar"), U.int_prim(123))
  |> KStatement.Interface.of_variable;

let __expression =
  (U.int_prim(123), U.int_prim(456))
  |> KExpression.Interface.of_equal_op
  |> U.as_int
  |> KStatement.Interface.of_effect;

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
          (U.as_untyped("foo"), U.int_prim(123)),
        )
    ),
    "enum"
    >: (
      () =>
        _assert_enum(
          Variable(
            "foo",
            Object([
              (
                "Verified",
                Function(
                  Some("Verified"),
                  ["a", "b"],
                  [
                    Return(
                      Array([
                        DotAccess(Identifier("foo"), "Verified"),
                        Identifier("a"),
                        Identifier("b"),
                      ])
                      |> Option.some,
                    ),
                  ],
                ),
              ),
              (
                "Unverified",
                Function(
                  Some("Unverified"),
                  ["a"],
                  [
                    Return(
                      Array([
                        DotAccess(Identifier("foo"), "Unverified"),
                        Identifier("a"),
                      ])
                      |> Option.some,
                    ),
                  ],
                ),
              ),
            ]),
          ),
          (
            U.as_untyped("foo"),
            [
              (
                U.as_untyped("Verified"),
                [
                  U.as_int(KTypeExpression.Interface.Integer),
                  U.as_string(KTypeExpression.Interface.String),
                ],
              ),
              (
                U.as_untyped("Unverified"),
                [U.as_string(KTypeExpression.Interface.String)],
              ),
            ],
          ),
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
            U.as_untyped("foo"),
            [(U.as_untyped("bar"), None, None) |> U.as_nil],
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
            U.as_untyped("foo"),
            [
              (U.as_untyped("bar"), None, Some(U.int_prim(123))) |> U.as_nil,
            ],
            (
              "bar" |> KExpression.Interface.of_identifier |> U.as_int,
              U.int_prim(5),
            )
            |> KExpression.Interface.of_add_op
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
            U.as_untyped("foo"),
            [],
            [
              (U.as_untyped("buzz"), U.int_prim(2))
              |> KStatement.Interface.of_variable
              |> U.as_nil,
              (
                "buzz" |> KExpression.Interface.of_identifier |> U.as_int,
                "buzz" |> KExpression.Interface.of_identifier |> U.as_int,
              )
              |> KExpression.Interface.of_divide_op
              |> U.as_float
              |> KStatement.Interface.of_effect
              |> U.as_float,
            ]
            |> KExpression.Interface.of_closure
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
                Variable(
                  "bar",
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
            U.as_untyped("foo"),
            [
              (U.as_untyped("bar"), None, Some(U.int_prim(123))) |> U.as_nil,
            ],
            [],
            (
              "bar" |> KExpression.Interface.of_identifier |> U.as_int,
              U.int_prim(5),
            )
            |> KExpression.Interface.of_add_op
            |> U.as_int,
          ),
        )
    ),
  ];
