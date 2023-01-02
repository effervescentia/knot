open Kore;
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

let _assert_style = (expected, actual) =>
  Alcotest.(
    check(
      Assert.Compare.expression(Target.Common),
      "javascript style matches",
      expected,
      actual |> Generator.gen_style,
    )
  );

let suite =
  "Generate.JavaScript_Generator | Expression"
  >::: [
    "boolean - true"
    >: (
      () => _assert_expression(Boolean(true), true |> A.of_bool |> A.of_prim)
    ),
    "boolean - false"
    >: (
      () =>
        _assert_expression(Boolean(false), false |> A.of_bool |> A.of_prim)
    ),
    "string - no special characters"
    >: (
      () =>
        _assert_expression(
          String("hello world"),
          "hello world" |> A.of_string |> A.of_prim,
        )
    ),
    "string - escaped quotation marks"
    >: (
      () =>
        _assert_expression(
          String("escaped quotes (\")"),
          "escaped quotes (\")" |> A.of_string |> A.of_prim,
        )
    ),
    "jsx - render empty tag"
    >: (
      () =>
        _assert_expression(
          FunctionCall(
            DotAccess(DotAccess(Identifier("$knot"), "jsx"), "createTag"),
            [String("foo")],
          ),
          ("foo" |> U.as_view([], Valid(Nil)), [], [], [])
          |> A.of_element_tag
          |> A.of_ksx,
        )
    ),
    "jsx - render tag with attributes"
    >: (
      () =>
        _assert_expression(
          FunctionCall(
            DotAccess(DotAccess(Identifier("$knot"), "jsx"), "createTag"),
            [String("foo"), Object([("zip", String("zap"))])],
          ),
          (
            "foo" |> U.as_view([], Valid(Nil)),
            [],
            [
              (
                U.as_untyped("zip"),
                "zap" |> A.of_string |> A.of_prim |> U.as_string |> Option.some,
              )
              |> U.as_untyped,
            ],
            [],
          )
          |> A.of_element_tag
          |> A.of_ksx,
        )
    ),
    "jsx - render component"
    >: (
      () =>
        _assert_expression(
          FunctionCall(
            DotAccess(DotAccess(Identifier("$knot"), "jsx"), "createTag"),
            [Identifier("Foo")],
          ),
          ("Foo" |> U.as_view([], Valid(Element)), [], [], [])
          |> A.of_component_tag
          |> A.of_ksx,
        )
    ),
    "jsx - render component with styles"
    >: (
      () =>
        _assert_expression(
          FunctionCall(
            DotAccess(DotAccess(Identifier("$knot"), "jsx"), "createTag"),
            [
              Identifier("Foo"),
              Object([
                (
                  "className",
                  FunctionCall(
                    DotAccess(
                      DotAccess(Identifier("$knot"), "style"),
                      "classes",
                    ),
                    [
                      FunctionCall(
                        DotAccess(Identifier("bar"), "getClass"),
                        [],
                      ),
                      FunctionCall(
                        DotAccess(
                          iife([
                            Variable(
                              "$",
                              DotAccess(
                                DotAccess(Identifier("$knot"), "style"),
                                "styleExpressionPlugin",
                              ),
                            ),
                            Variable(
                              "$rules$",
                              DotAccess(
                                DotAccess(Identifier("$knot"), "style"),
                                "styleRulePlugin",
                              ),
                            ),
                            Return(
                              FunctionCall(
                                DotAccess(
                                  DotAccess(Identifier("$knot"), "style"),
                                  "createStyle",
                                ),
                                [
                                  Object([
                                    (
                                      "color",
                                      FunctionCall(
                                        DotAccess(
                                          Identifier("$rules$"),
                                          "color",
                                        ),
                                        [String("red")],
                                      ),
                                    ),
                                  ]),
                                ],
                              )
                              |> Option.some,
                            ),
                          ]),
                          "getClass",
                        ),
                        [],
                      ),
                    ],
                  ),
                ),
              ]),
            ],
          ),
          (
            "Foo" |> U.as_view([], Valid(Element)),
            [
              "bar" |> A.of_id |> U.as_style,
              [
                (U.as_string("color"), U.string_prim("red")) |> U.as_untyped,
              ]
              |> A.of_style
              |> U.as_style,
            ],
            [],
            [],
          )
          |> A.of_component_tag
          |> A.of_ksx,
        )
    ),
    "jsx - deeply nested tags"
    >: (
      () =>
        _assert_expression(
          FunctionCall(
            DotAccess(DotAccess(Identifier("$knot"), "jsx"), "createTag"),
            [
              String("foo"),
              Null,
              FunctionCall(
                DotAccess(
                  DotAccess(Identifier("$knot"), "jsx"),
                  "createTag",
                ),
                [
                  Identifier("Bar"),
                  Null,
                  FunctionCall(
                    DotAccess(
                      DotAccess(Identifier("$knot"), "jsx"),
                      "createTag",
                    ),
                    [String("fizz")],
                  ),
                ],
              ),
            ],
          ),
          (
            "foo" |> U.as_view([], Valid(Nil)),
            [],
            [],
            [
              (
                "Bar" |> U.as_view([], Valid(Element)),
                [],
                [],
                [
                  ("fizz" |> U.as_view([], Valid(Nil)), [], [], [])
                  |> A.of_element_tag
                  |> A.of_node
                  |> U.as_untyped,
                ],
              )
              |> A.of_component_tag
              |> A.of_node
              |> U.as_untyped,
            ],
          )
          |> A.of_element_tag
          |> A.of_ksx,
        )
    ),
    "null" >: (() => _assert_expression(Null, A.nil |> A.of_prim)),
    "identifier"
    >: (() => _assert_expression(Identifier("fooBar"), A.of_id("fooBar"))),
    "group"
    >: (
      () =>
        _assert_expression(
          Group(Number("123")),
          123 |> U.int_prim |> A.of_group,
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
            (U.int_prim(123), U.int_prim(456))
            |> A.of_eq_op
            |> U.as_int
            |> A.of_effect
            |> U.as_int,
            (U.int_prim(678), U.int_prim(910))
            |> A.of_add_op
            |> U.as_int
            |> A.of_effect
            |> U.as_int,
          ]
          |> A.of_closure,
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
          [(U.as_untyped("foo"), U.int_prim(456)) |> A.of_var |> U.as_nil]
          |> A.of_closure,
        )
    ),
    "dot access"
    >: (
      () =>
        _assert_expression(
          DotAccess(Identifier("foo"), "bar"),
          (
            "foo"
            |> A.of_id
            |> U.as_struct([("bar", (Valid(String), true))]),
            U.as_untyped("bar"),
          )
          |> A.of_dot_access,
        )
    ),
    "style binding"
    >: (
      () =>
        _assert_expression(
          FunctionCall(
            DotAccess(DotAccess(Identifier("$knot"), "jsx"), "bindStyle"),
            [Identifier("foo"), Identifier("bar")],
          ),
          (
            "foo" |> A.of_id |> U.as_view([], Valid(Element)),
            "bar" |> A.of_id |> U.as_style,
          )
          |> A.of_component_bind_style,
        )
    ),
    "function call"
    >: (
      () =>
        _assert_expression(
          FunctionCall(Identifier("foo"), [Identifier("bar")]),
          (
            "foo"
            |> A.of_id
            |> U.as_function([Valid(String)], Valid(Boolean)),
            ["bar" |> A.of_id |> U.as_string],
          )
          |> A.of_func_call,
        )
    ),
    "binary operation - logical and"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("&&", Boolean(true), Boolean(false))),
          (LogicalAnd, U.bool_prim(true), U.bool_prim(false)),
        )
    ),
    "binary operation - logical or"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("||", Boolean(true), Boolean(false))),
          (LogicalOr, U.bool_prim(true), U.bool_prim(false)),
        )
    ),
    "binary operation - less than or equal"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("<=", Number("123"), Number("456"))),
          (LessOrEqual, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "binary operation - less than"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("<", Number("123"), Number("456"))),
          (LessThan, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "binary operation - greater than or equal"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp(">=", Number("123"), Number("456"))),
          (GreaterOrEqual, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "binary operation - greater than"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp(">", Number("123"), Number("456"))),
          (GreaterThan, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "binary operation - equal"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("===", Number("123"), Number("456"))),
          (Equal, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "binary operation - unequal"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("!==", Number("123"), Number("456"))),
          (Unequal, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "binary operation - add"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("+", Number("123"), Number("456"))),
          (Add, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "binary operation - subtract"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("-", Number("123"), Number("456"))),
          (Subtract, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "binary operation - multiply"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("*", Number("123"), Number("456"))),
          (Multiply, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "binary operation - divide"
    >: (
      () =>
        _assert_binary_op(
          Group(BinaryOp("/", Number("123"), Number("456"))),
          (Divide, U.int_prim(123), U.int_prim(456)),
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
          (Exponent, U.int_prim(123), U.int_prim(456)),
        )
    ),
    "unary operation - logical negation"
    >: (
      () =>
        _assert_unary_op(
          UnaryOp("!", Group(Boolean(true))),
          (Not, U.bool_prim(true)),
        )
    ),
    "unary operation - positive"
    >: (
      () =>
        _assert_unary_op(
          FunctionCall(
            DotAccess(Identifier("Math"), "abs"),
            [Number("123")],
          ),
          (Positive, U.int_prim(123)),
        )
    ),
    "unary operation - negative"
    >: (
      () =>
        _assert_unary_op(
          UnaryOp("-", Group(Number("123"))),
          (Negative, U.int_prim(123)),
        )
    ),
    "style rules"
    >: (
      () =>
        _assert_style(
          FunctionCall(
            Group(
              Function(
                None,
                [],
                [
                  Variable(
                    "$",
                    DotAccess(
                      DotAccess(Identifier("$knot"), "style"),
                      "styleExpressionPlugin",
                    ),
                  ),
                  Variable(
                    "$rules$",
                    DotAccess(
                      DotAccess(Identifier("$knot"), "style"),
                      "styleRulePlugin",
                    ),
                  ),
                  Return(
                    Some(
                      FunctionCall(
                        DotAccess(
                          DotAccess(Identifier("$knot"), "style"),
                          "createStyle",
                        ),
                        [
                          Object([
                            (
                              "height",
                              FunctionCall(
                                DotAccess(Identifier("$rules$"), "height"),
                                [Number("2")],
                              ),
                            ),
                            (
                              "width",
                              FunctionCall(
                                DotAccess(Identifier("$rules$"), "width"),
                                [Number("10")],
                              ),
                            ),
                          ]),
                        ],
                      ),
                    ),
                  ),
                ],
              ),
            ),
            [],
          ),
          [
            (U.as_int("height"), U.int_prim(2)) |> U.as_untyped,
            (U.as_int("width"), U.int_prim(10)) |> U.as_untyped,
          ],
        )
    ),
  ];
