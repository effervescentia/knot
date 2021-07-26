open Kore;
open Util.ResultUtil;
open Generate.JavaScript_AST;

module Generator = Generate.JavaScript_Generator;
module Formatter = Generate.JavaScript_Formatter;

let __resolved = "../foo/bar";
let __program = [
  (
    "foo/bar" |> of_internal,
    ["Foo" |> of_public |> as_lexeme |> of_main_import],
  )
  |> of_import,
  (
    "ABC" |> of_public |> as_lexeme |> of_named_export,
    123 |> int_prim |> of_const |> as_int,
  )
  |> of_decl,
];

module Compare = {
  open Alcotest;

  let expression = (module_type: Target.module_t) =>
    testable(
      pp =>
        Formatter.fmt_expression(module_type)
        % Pretty.to_string
        % Format.pp_print_string(pp),
      (==),
    );

  let statement = (module_type: Target.module_t) =>
    testable(
      pp =>
        Formatter.fmt_statement(module_type)
        % Pretty.to_string
        % Format.pp_print_string(pp),
      (==),
    );
};

let _assert_expression = (~module_type=Target.Common) =>
  Alcotest.(
    check(Compare.expression(module_type), "javascript expression matches")
  );

let _assert_statement = (~module_type=Target.Common) =>
  Alcotest.(
    check(Compare.statement(module_type), "javascript statement matches")
  );
let _assert_statement_list = (~module_type=Target.Common) =>
  Alcotest.(
    check(
      list(Compare.statement(module_type)),
      "javascript statement matches",
    )
  );

let suite =
  "Generate.JavaScript_Generator"
  >::: [
    "number()"
    >: (
      () =>
        [
          (Number("123"), 123 |> Int64.of_int |> of_int),
          (Number("9223372036854775807"), Int64.max_int |> of_int),
          (Number("-9223372036854775808"), Int64.min_int |> of_int),
          (Number("0"), (0.0, 0) |> of_float),
          (Number("1"), (1.0, 0) |> of_float),
          (Number("12345"), (12345.0, 5) |> of_float),
          (Number("4.56"), (4.56, 3) |> of_float),
          (Number("1234.56"), (1234.56, 6) |> of_float),
          (Number("0.123"), (0.123, 3) |> of_float),
          (Number("0.000456"), (0.000456, 6) |> of_float),
          (Number("1.79769313486e+308"), (Float.max_float, 12) |> of_float),
          (Number("2.22507385851e-308"), (Float.min_float, 12) |> of_float),
        ]
        |> List.map(Tuple.map_snd2(Generator.gen_number))
        |> Assert.(test_many(_assert_expression))
    ),
    "expression()"
    >: (
      () =>
        [
          (Boolean(true), true |> of_bool |> as_bool |> of_prim),
          (Boolean(false), false |> of_bool |> as_bool |> of_prim),
          (
            String("hello world"),
            "hello world" |> of_string |> as_string |> of_prim,
          ),
          (
            String("escaped quotes (\")"),
            "escaped quotes (\")" |> of_string |> as_string |> of_prim,
          ),
          (Null, nil |> as_nil |> of_prim),
          (
            Identifier("fooBar"),
            "fooBar" |> of_public |> as_unknown |> of_id,
          ),
          (Group(Number("123")), 123 |> int_prim |> of_group),
          (
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
              |> of_expr,
              (678 |> int_prim, 910 |> int_prim)
              |> of_add_op
              |> as_int
              |> of_expr,
            ]
            |> of_closure,
          ),
          (
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
            [("foo" |> of_public |> as_lexeme, 456 |> int_prim) |> of_var]
            |> of_closure,
          ),
        ]
        |> List.map(Tuple.map_snd2(Generator.gen_expression))
        |> Assert.(test_many(_assert_expression))
    ),
    "statement()"
    >: (
      () =>
        [
          (
            [Variable("fooBar", Number("123"))],
            ("fooBar" |> of_public |> as_lexeme, 123 |> int_prim)
            |> of_var
            |> Generator.gen_statement,
          ),
          (
            [Variable("fooBar", Number("123")), Return(Some(Null))],
            ("fooBar" |> of_public |> as_lexeme, 123 |> int_prim)
            |> of_var
            |> Generator.gen_statement(~is_last=true),
          ),
          (
            [
              Expression(
                Group(BinaryOp("===", Number("123"), Number("456"))),
              ),
            ],
            (123 |> int_prim, 456 |> int_prim)
            |> of_eq_op
            |> as_int
            |> of_expr
            |> Generator.gen_statement,
          ),
          (
            [
              Return(
                Some(
                  Group(BinaryOp("===", Number("123"), Number("456"))),
                ),
              ),
            ],
            (123 |> int_prim, 456 |> int_prim)
            |> of_eq_op
            |> as_int
            |> of_expr
            |> Generator.gen_statement(~is_last=true),
          ),
        ]
        |> Assert.(test_many(_assert_statement_list))
    ),
    "binary_op()"
    >: (
      () => {
        [
          (
            Group(BinaryOp("&&", Boolean(true), Boolean(false))),
            Generator.gen_binary_op(
              LogicalAnd,
              true |> bool_prim,
              false |> bool_prim,
            ),
          ),
          (
            Group(BinaryOp("||", Boolean(true), Boolean(false))),
            Generator.gen_binary_op(
              LogicalOr,
              true |> bool_prim,
              false |> bool_prim,
            ),
          ),
          (
            Group(BinaryOp("<=", Number("123"), Number("456"))),
            Generator.gen_binary_op(
              LessOrEqual,
              123 |> int_prim,
              456 |> int_prim,
            ),
          ),
          (
            Group(BinaryOp("<", Number("123"), Number("456"))),
            Generator.gen_binary_op(
              LessThan,
              123 |> int_prim,
              456 |> int_prim,
            ),
          ),
          (
            Group(BinaryOp(">=", Number("123"), Number("456"))),
            Generator.gen_binary_op(
              GreaterOrEqual,
              123 |> int_prim,
              456 |> int_prim,
            ),
          ),
          (
            Group(BinaryOp(">", Number("123"), Number("456"))),
            Generator.gen_binary_op(
              GreaterThan,
              123 |> int_prim,
              456 |> int_prim,
            ),
          ),
          (
            Group(BinaryOp("===", Number("123"), Number("456"))),
            Generator.gen_binary_op(Equal, 123 |> int_prim, 456 |> int_prim),
          ),
          (
            Group(BinaryOp("!==", Number("123"), Number("456"))),
            Generator.gen_binary_op(
              Unequal,
              123 |> int_prim,
              456 |> int_prim,
            ),
          ),
          (
            Group(BinaryOp("+", Number("123"), Number("456"))),
            Generator.gen_binary_op(Add, 123 |> int_prim, 456 |> int_prim),
          ),
          (
            Group(BinaryOp("-", Number("123"), Number("456"))),
            Generator.gen_binary_op(
              Subtract,
              123 |> int_prim,
              456 |> int_prim,
            ),
          ),
          (
            Group(BinaryOp("*", Number("123"), Number("456"))),
            Generator.gen_binary_op(
              Multiply,
              123 |> int_prim,
              456 |> int_prim,
            ),
          ),
          (
            Group(BinaryOp("/", Number("123"), Number("456"))),
            Generator.gen_binary_op(Divide, 123 |> int_prim, 456 |> int_prim),
          ),
          (
            FunctionCall(
              DotAccess(Identifier("Math"), "pow"),
              [Number("123"), Number("456")],
            ),
            Generator.gen_binary_op(
              Exponent,
              123 |> int_prim,
              456 |> int_prim,
            ),
          ),
        ]
        |> Assert.(test_many(_assert_expression));
      }
    ),
    "unary_op()"
    >: (
      () =>
        [
          (
            UnaryOp("!", Group(Boolean(true))),
            Generator.gen_unary_op(Not, true |> bool_prim),
          ),
          (
            UnaryOp("+", Group(Number("123"))),
            Generator.gen_unary_op(Positive, 123 |> int_prim),
          ),
          (
            UnaryOp("-", Group(Number("123"))),
            Generator.gen_unary_op(Negative, 123 |> int_prim),
          ),
        ]
        |> Assert.(test_many(_assert_expression))
    ),
    "jsx()"
    >: (
      () =>
        [
          (
            FunctionCall(
              DotAccess(DotAccess(Identifier("$knot"), "jsx"), "createTag"),
              [String("Foo")],
            ),
            ("Foo" |> of_public |> as_lexeme, [], []) |> of_tag,
          ),
          (
            FunctionCall(
              DotAccess(
                DotAccess(Identifier("$knot"), "jsx"),
                "createFragment",
              ),
              [],
            ),
            [] |> of_frag,
          ),
        ]
        |> List.map(Tuple.map_snd2(Generator.gen_jsx))
        |> Assert.(test_many(_assert_expression))
    ),
    "jsx_child()"
    >: (
      () =>
        [
          (
            FunctionCall(
              DotAccess(DotAccess(Identifier("$knot"), "jsx"), "createTag"),
              [String("Foo")],
            ),
            ("Foo" |> of_public |> as_lexeme, [], [])
            |> of_tag
            |> as_element
            |> of_node,
          ),
          (
            FunctionCall(
              DotAccess(DotAccess(Identifier("$knot"), "jsx"), "createTag"),
              [
                String("Foo"),
                Object([
                  ("bar", Identifier("fizz")),
                  ("foo", Identifier("foo")),
                ]),
              ],
            ),
            (
              "Foo" |> of_public |> as_lexeme,
              [
                ("foo" |> of_public |> as_lexeme, None)
                |> of_prop
                |> as_unknown,
                (
                  "bar" |> of_public |> as_lexeme,
                  Some(
                    "fizz" |> of_public |> as_unknown |> of_id |> as_unknown,
                  ),
                )
                |> of_prop
                |> as_unknown,
              ],
              [],
            )
            |> of_tag
            |> as_element
            |> of_node,
          ),
          (
            FunctionCall(
              DotAccess(DotAccess(Identifier("$knot"), "jsx"), "createTag"),
              [
                String("Foo"),
                Null,
                FunctionCall(
                  DotAccess(
                    DotAccess(Identifier("$knot"), "jsx"),
                    "createTag",
                  ),
                  [String("Bar"), Null, String("fizz")],
                ),
              ],
            ),
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                (
                  "Bar" |> of_public |> as_lexeme,
                  [],
                  ["fizz" |> as_string |> of_text |> as_string],
                )
                |> of_tag
                |> as_element
                |> of_node
                |> as_element,
              ],
            )
            |> of_tag
            |> as_element
            |> of_node,
          ),
          (String("Hello World!"), "Hello World!" |> as_string |> of_text),
          (Number("123"), 123 |> int_prim |> of_inline_expr),
        ]
        |> List.map(Tuple.map_snd2(Generator.gen_jsx_child))
        |> Assert.(test_many(_assert_expression))
    ),
    "jsx_attrs()"
    >: (
      () =>
        [
          (
            Object([("foo", Identifier("foo"))]),
            [
              ("foo" |> of_public |> as_lexeme, None) |> of_prop |> as_unknown,
            ],
          ),
          (
            Object([("foo", Identifier("bar"))]),
            [
              (
                "foo" |> of_public |> as_lexeme,
                Some("bar" |> of_public |> as_unknown |> of_id |> as_unknown),
              )
              |> of_prop
              |> as_unknown,
            ],
          ),
          (
            Object([("className", String(".foo"))]),
            [
              ("foo" |> of_public |> as_lexeme, None)
              |> of_jsx_class
              |> as_unknown,
            ],
          ),
          (
            Object([
              (
                "className",
                BinaryOp(
                  "+",
                  Group(
                    Ternary(
                      Group(BinaryOp(">", Number("123"), Number("456"))),
                      String(".foo"),
                      String(""),
                    ),
                  ),
                  String(".bar"),
                ),
              ),
            ]),
            [
              ("bar" |> of_public |> as_lexeme, None)
              |> of_jsx_class
              |> as_unknown,
              (
                "foo" |> of_public |> as_lexeme,
                Some(
                  (123 |> int_prim, 456 |> int_prim) |> of_gt_op |> as_bool,
                ),
              )
              |> of_jsx_class
              |> as_unknown,
            ],
          ),
          (
            Object([("id", String("foo"))]),
            ["foo" |> of_public |> as_lexeme |> of_jsx_id |> as_unknown],
          ),
        ]
        |> List.map(Tuple.map_snd2(Generator.gen_jsx_attrs))
        |> Assert.(test_many(_assert_expression))
    ),
    "constant()"
    >: (
      () =>
        [
          (
            Variable("foo", Number("123")),
            Generator.gen_constant(
              "foo" |> of_public |> as_lexeme,
              123 |> int_prim,
            ),
          ),
        ]
        |> Assert.(test_many(_assert_statement))
    ),
    "function()"
    >: (
      () =>
        [
          (
            Expression(
              Function(
                Some("foo"),
                ["bar"],
                [Return(Some(Number("123")))],
              ),
            ),
            Generator.gen_function(
              "foo" |> of_public |> as_lexeme,
              [
                {
                  name: "bar" |> of_public |> as_lexeme,
                  default: None,
                  type_: None,
                }
                |> as_nil,
              ],
              123 |> int_prim,
            ),
          ),
          (
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
            Generator.gen_function(
              "foo" |> of_public |> as_lexeme,
              [
                {
                  name: "bar" |> of_public |> as_lexeme,
                  default: Some(123 |> int_prim),
                  type_: None,
                }
                |> as_nil,
              ],
              ("bar" |> of_public |> as_int |> of_id |> as_int, 5 |> int_prim)
              |> of_add_op
              |> as_int,
            ),
          ),
          (
            Expression(
              Function(
                Some("foo"),
                [],
                [
                  Variable("buzz", Number("2")),
                  Return(
                    Some(
                      Group(
                        BinaryOp(
                          "/",
                          Identifier("buzz"),
                          Identifier("buzz"),
                        ),
                      ),
                    ),
                  ),
                ],
              ),
            ),
            Generator.gen_function(
              "foo" |> of_public |> as_lexeme,
              [],
              [
                ("buzz" |> of_public |> as_lexeme, 2 |> int_prim) |> of_var,
                (
                  "buzz" |> of_public |> as_int |> of_id |> as_int,
                  "buzz" |> of_public |> as_int |> of_id |> as_int,
                )
                |> of_div_op
                |> as_float
                |> of_expr,
              ]
              |> of_closure
              |> as_float,
            ),
          ),
        ]
        |> Assert.(test_many(_assert_statement))
    ),
    "declaration()"
    >: (
      () =>
        [
          (
            [Variable("foo", Number("123")), Export("foo", None)],
            Generator.gen_declaration(
              "foo" |> of_public |> as_lexeme,
              123 |> int_prim |> of_const |> as_int,
            ),
          ),
          (
            [Variable("_foo", Number("123"))],
            Generator.gen_declaration(
              "foo" |> of_private |> as_lexeme,
              123 |> int_prim |> of_const |> as_int,
            ),
          ),
        ]
        |> Assert.(test_many(_assert_statement_list))
    ),
    "generate() - empty module"
    >: (
      () =>
        _assert_statement_list(
          [EmptyExport],
          Generator.generate(_ => "", []),
        )
    ),
    "generate() - with imports and exports"
    >: (
      () =>
        _assert_statement_list(
          [
            DefaultImport("@knot/runtime", "$knot"),
            Import("../foo/bar", [("main", Some("Foo"))]),
            Variable("ABC", Number("123")),
            Export("ABC", None),
          ],
          Generator.generate(
            path => {
              Assert.namespace("foo/bar" |> of_internal, path);
              __resolved;
            },
            __program,
          ),
        )
    ),
  ];
