open Kore;
open AST.Raw.Util;
open Reference;
open Util;

module Formatter = Grammar.Formatter;

let suite =
  "Grammar.Formatter"
  >::: [
    "fmt_binary_op()"
    >: (
      () =>
        AST.[
          ("&&", LogicalAnd),
          ("||", LogicalOr),
          ("+", Add),
          ("-", Subtract),
          ("/", Divide),
          ("*", Multiply),
          ("<=", LessOrEqual),
          ("<", LessThan),
          (">=", GreaterOrEqual),
          (">", GreaterThan),
          ("==", Equal),
          ("!=", Unequal),
          ("^", Exponent),
        ]
        |> List.map(Tuple.map_snd2(fmt_binary_op % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_unary_op()"
    >: (
      () =>
        AST.[("!", Not), ("+", Positive), ("-", Negative)]
        |> List.map(Tuple.map_snd2(fmt_unary_op % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_id()"
    >: Identifier.(
         () =>
           [("fooBar", Public("fooBar")), ("_fooBar", Private("fooBar"))]
           |> List.map(Tuple.map_snd2(fmt_id % Pretty.to_string))
           |> Assert.(test_many(string))
       ),
    "fmt_ns()"
    >: (
      () =>
        Namespace.[
          ("\"@/fooBar\"", Internal("fooBar")),
          ("\"fooBar\"", External("fooBar")),
        ]
        |> List.map(Tuple.map_snd2(fmt_ns % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_num()"
    >: (
      () =>
        [
          ("123", 123 |> Int64.of_int |> to_int),
          ("9223372036854775807", Int64.max_int |> to_int),
          ("-9223372036854775808", Int64.min_int |> to_int),
          ("123.456", (123.456, 3) |> to_float),
        ]
        |> List.map(Tuple.map_snd2(fmt_num % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_prim()"
    >: (
      () =>
        [
          ("123", 123 |> Int64.of_int |> to_int |> to_num),
          ("true", true |> to_bool),
          ("false", false |> to_bool),
          ("nil", nil),
          ("\"foo bar\"", "foo bar" |> to_string),
        ]
        |> List.map(Tuple.map_snd2(fmt_prim % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_expression()"
    >: (
      () =>
        [
          ("nil", nil |> as_nil |> to_prim),
          ("fooBar", "fooBar" |> to_public |> as_lexeme |> to_id),
          ("123", 123 |> int_prim |> to_group),
          ("123", 123 |> int_prim |> to_group |> as_int |> to_group),
          (
            "(123 + 456)",
            (123 |> int_prim, 456 |> int_prim)
            |> to_add_op
            |> as_int
            |> to_group,
          ),
          (
            "(123 + 456)",
            (123 |> int_prim, 456 |> int_prim)
            |> to_add_op
            |> as_int
            |> to_group
            |> as_int
            |> to_group,
          ),
          (
            "{
  true;
  false;
  nil;
}",
            [
              true |> bool_prim |> to_expr,
              false |> bool_prim |> to_expr,
              nil_prim |> to_expr,
            ]
            |> to_closure,
          ),
          ("!true", true |> bool_prim |> to_not_op),
          (
            "true || false",
            (true |> bool_prim, false |> bool_prim) |> to_or_op,
          ),
          (
            "<Foo>
  bar
</Foo>",
            (
              "Foo" |> to_public |> as_lexeme,
              [],
              ["bar" |> as_lexeme |> to_text |> as_lexeme],
            )
            |> to_tag
            |> as_lexeme
            |> to_jsx,
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_expression % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_jsx()"
    >: (
      () =>
        [
          ("<Foo />", ("Foo" |> to_public |> as_lexeme, [], []) |> to_tag),
          ("<></>", [] |> to_frag),
          (
            "<Foo #bar .fizz buzz />",
            (
              "Foo" |> to_public |> as_lexeme,
              [
                "bar" |> to_public |> as_lexeme |> to_jsx_id |> as_lexeme,
                ("fizz" |> to_public |> as_lexeme, None)
                |> to_jsx_class
                |> as_lexeme,
                ("buzz" |> to_public |> as_lexeme, None)
                |> to_prop
                |> as_lexeme,
              ],
              [],
            )
            |> to_tag,
          ),
          (
            "<Foo>
  bar
</Foo>",
            (
              "Foo" |> to_public |> as_lexeme,
              [],
              ["bar" |> as_lexeme |> to_text |> as_lexeme],
            )
            |> to_tag,
          ),
          (
            "<Foo>
  {1 + 5}
</Foo>",
            (
              "Foo" |> to_public |> as_lexeme,
              [],
              [
                (1 |> int_prim, 5 |> int_prim)
                |> to_add_op
                |> as_int
                |> to_inline_expr
                |> as_lexeme,
              ],
            )
            |> to_tag,
          ),
          (
            "<Foo>
  <Bar>
    fizzbuzz
  </Bar>
</Foo>",
            (
              "Foo" |> to_public |> as_lexeme,
              [],
              [
                (
                  "Bar" |> to_public |> as_lexeme,
                  [],
                  ["fizzbuzz" |> as_lexeme |> to_text |> as_lexeme],
                )
                |> to_tag
                |> as_lexeme
                |> to_node
                |> as_lexeme,
              ],
            )
            |> to_tag,
          ),
          (
            "<Foo>
  <Bar />
  {nil}
  Hello, World!
</Foo>",
            (
              "Foo" |> to_public |> as_lexeme,
              [],
              [
                ("Bar" |> to_public |> as_lexeme, [], [])
                |> to_tag
                |> as_lexeme
                |> to_node
                |> as_lexeme,
                nil_prim |> to_inline_expr |> as_lexeme,
                "Hello, World!" |> as_lexeme |> to_text |> as_lexeme,
              ],
            )
            |> to_tag,
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_jsx % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_jsx_attr()"
    >: (
      () =>
        [
          (
            "fizz=buzz",
            (
              "fizz" |> to_public |> as_lexeme,
              Some("buzz" |> to_public |> as_lexeme |> to_id |> as_weak(0)),
            )
            |> to_prop,
          ),
          (
            "fizz=123",
            ("fizz" |> to_public |> as_lexeme, Some(123 |> int_prim))
            |> to_prop,
          ),
          (
            "fizz=(1 + 2)",
            (
              "fizz" |> to_public |> as_lexeme,
              Some((1 |> int_prim, 2 |> int_prim) |> to_add_op |> as_int),
            )
            |> to_prop,
          ),
          (
            "fizz=(-1)",
            (
              "fizz" |> to_public |> as_lexeme,
              Some(1 |> int_prim |> to_neg_op |> as_int),
            )
            |> to_prop,
          ),
          (
            "fizz=true",
            (
              "fizz" |> to_public |> as_lexeme,
              Some(
                true |> bool_prim |> to_group |> as_bool |> to_group |> as_bool,
              ),
            )
            |> to_prop,
          ),
          (
            "fizz=(1 + 2)",
            (
              "fizz" |> to_public |> as_lexeme,
              Some(
                (1 |> int_prim, 2 |> int_prim)
                |> to_add_op
                |> as_int
                |> to_group
                |> as_int,
              ),
            )
            |> to_prop,
          ),
          (
            "fizz={
  true;
  false;
}",
            (
              "fizz" |> to_public |> as_lexeme,
              Some(
                [true |> bool_prim |> to_expr, false |> bool_prim |> to_expr]
                |> to_closure
                |> as_bool,
              ),
            )
            |> to_prop,
          ),
          (
            "fizz=<Buzz />",
            (
              "fizz" |> to_public |> as_lexeme,
              Some(
                ("Buzz" |> to_public |> as_lexeme, [], [])
                |> to_tag
                |> as_lexeme
                |> to_jsx
                |> as_element,
              ),
            )
            |> to_prop,
          ),
          (
            "fizz=(<Buzz>
  <Foo />
</Buzz>)",
            (
              "fizz" |> to_public |> as_lexeme,
              Some(
                (
                  "Buzz" |> to_public |> as_lexeme,
                  [],
                  [
                    ("Foo" |> to_public |> as_lexeme, [], [])
                    |> to_tag
                    |> as_lexeme
                    |> to_node
                    |> as_lexeme,
                  ],
                )
                |> to_tag
                |> as_lexeme
                |> to_jsx
                |> as_element,
              ),
            )
            |> to_prop,
          ),
          ("buzz", ("buzz" |> to_public |> as_lexeme, None) |> to_prop),
          (
            ".fizz=true",
            ("fizz" |> to_public |> as_lexeme, Some(true |> bool_prim))
            |> to_jsx_class,
          ),
          (
            ".fizz",
            ("fizz" |> to_public |> as_lexeme, None) |> to_jsx_class,
          ),
          ("#bar", "bar" |> to_public |> as_lexeme |> to_jsx_id),
        ]
        |> List.map(Tuple.map_snd2(fmt_jsx_attr % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_statement()"
    >: (
      () =>
        [
          ("nil;", nil |> as_nil |> to_prim |> as_nil |> to_expr),
          (
            "let foo = nil;",
            (
              "foo" |> to_public |> as_lexeme,
              nil |> as_nil |> to_prim |> as_nil,
            )
            |> to_var,
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_statement % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_declaration() - constant"
    >: (
      () =>
        [
          (
            "const foo = nil;\n",
            (
              "foo" |> to_public |> as_lexeme,
              nil |> as_nil |> to_prim |> as_nil |> to_const,
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_declaration % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_declaration() - function"
    >: (
      () =>
        [
          (
            "func foo(bar, fizz = 3) -> bar + fizz;\n",
            (
              "foo" |> to_public |> as_lexeme,
              (
                [
                  (
                    {name: "bar" |> to_public |> as_lexeme, default: None},
                    Type.K_Strong(K_Integer),
                  ),
                  (
                    {
                      name: "fizz" |> to_public |> as_lexeme,
                      default: Some(3 |> int_prim),
                    },
                    Type.K_Strong(K_Integer),
                  ),
                ],
                (
                  "bar" |> to_public |> as_lexeme |> to_id |> as_int,
                  "fizz" |> to_public |> as_lexeme |> to_id |> as_int,
                )
                |> to_add_op
                |> as_int,
              )
              |> of_func,
            ),
          ),
          (
            "func buzz -> {
  let zip = 3;
  let zap = 4;
  zip * zap;
}
",
            (
              "buzz" |> to_public |> as_lexeme,
              (
                [],
                [
                  ("zip" |> to_public |> as_lexeme, 3 |> int_prim) |> to_var,
                  ("zap" |> to_public |> as_lexeme, 4 |> int_prim) |> to_var,
                  (
                    "zip" |> to_public |> as_lexeme |> to_id |> as_int,
                    "zap" |> to_public |> as_lexeme |> to_id |> as_int,
                  )
                  |> to_mult_op
                  |> as_int
                  |> to_expr,
                ]
                |> to_closure
                |> as_int,
              )
              |> to_func,
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_declaration % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_declarations()"
    >: (
      () =>
        [
          (
            "const DEF = true;
const ABC = 123;
",
            [
              (
                "DEF" |> to_public |> as_lexeme |> to_named_export,
                true |> bool_prim |> to_const,
              )
              |> to_decl,
              (
                "ABC" |> to_public |> as_lexeme |> to_named_export,
                123 |> int_prim |> to_const,
              )
              |> to_decl,
            ],
          ),
        ]
        |> List.map(
             Tuple.map_snd2(
               fmt_declarations % Pretty.concat % Pretty.to_string,
             ),
           )
        |> Assert.(test_many(string))
    ),
    "fmt_imports()"
    >: (
      () =>
        [
          (
            "import bar, { Bar, Foo as foo } from \"bar\";
import Fizz from \"buzz\";
",
            [
              (
                "buzz" |> to_external,
                ["Fizz" |> to_public |> as_lexeme |> to_main_import],
              )
              |> to_import,
              (
                "bar" |> to_external,
                [
                  "bar" |> to_public |> as_lexeme |> to_main_import,
                  (
                    "Foo" |> to_public |> as_lexeme,
                    Some("foo" |> to_public |> as_lexeme),
                  )
                  |> to_named_import,
                  ("Bar" |> to_public |> as_lexeme, None) |> to_named_import,
                ],
              )
              |> to_import,
            ],
          ),
          (
            "import Fizz from \"buzz\";

import Foo from \"@/bar\";
",
            [
              (
                "bar" |> to_internal,
                ["Foo" |> to_public |> as_lexeme |> to_main_import],
              )
              |> to_import,
              (
                "buzz" |> to_external,
                ["Fizz" |> to_public |> as_lexeme |> to_main_import],
              )
              |> to_import,
            ],
          ),
        ]
        |> List.map(
             Tuple.map_snd2(fmt_imports % Pretty.concat % Pretty.to_string),
           )
        |> Assert.(test_many(string))
    ),
    "format()"
    >: (
      () =>
        [
          ("", []),
          (
            "import Foo from \"bar\";\n",
            [
              (
                "bar" |> to_external,
                ["Foo" |> to_public |> as_lexeme |> to_main_import],
              )
              |> to_import,
            ],
          ),
          (
            "const ABC = 123;\n",
            [
              (
                "ABC" |> to_public |> as_lexeme |> to_named_export,
                123 |> int_prim |> to_const,
              )
              |> to_decl,
            ],
          ),
          (
            "import Foo from \"bar\";

const ABC = 123;
",
            [
              (
                "bar" |> to_external,
                ["Foo" |> to_public |> as_lexeme |> to_main_import],
              )
              |> to_import,
              (
                "ABC" |> to_public |> as_lexeme |> to_named_export,
                123 |> int_prim |> to_const,
              )
              |> to_decl,
            ],
          ),
        ]
        |> List.map(Tuple.map_snd2(Formatter.format % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
  ];
