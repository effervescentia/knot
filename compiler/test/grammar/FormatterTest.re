open Kore;
open AST;
open Util;
open Reference;

module Formatter = Grammar.Formatter;

let suite =
  "Grammar.Formatter"
  >::: [
    "fmt_binary_op()"
    >: (
      () =>
        [
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
        [("!", Not), ("+", Positive), ("-", Negative)]
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
          ("123", 123 |> Int64.of_int |> of_int),
          ("9223372036854775807", Int64.max_int |> of_int),
          ("-9223372036854775808", Int64.min_int |> of_int),
          ("123.456", (123.456, 3) |> of_float),
        ]
        |> List.map(Tuple.map_snd2(fmt_num % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_prim()"
    >: (
      () =>
        [
          ("123", 123 |> Int64.of_int |> of_int |> of_num),
          ("true", true |> of_bool),
          ("false", false |> of_bool),
          ("nil", nil),
          ("\"foo bar\"", "foo bar" |> of_string),
        ]
        |> List.map(Tuple.map_snd2(fmt_prim % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_expression()"
    >: (
      () =>
        [
          ("nil", nil |> as_nil |> of_prim),
          ("fooBar", "fooBar" |> of_public |> as_lexeme |> of_id),
          ("123", 123 |> int_prim |> of_group),
          ("123", 123 |> int_prim |> of_group |> as_int |> of_group),
          (
            "(123 + 456)",
            (123 |> int_prim, 456 |> int_prim)
            |> of_add_op
            |> as_int
            |> of_group,
          ),
          (
            "(123 + 456)",
            (123 |> int_prim, 456 |> int_prim)
            |> of_add_op
            |> as_int
            |> of_group
            |> as_int
            |> of_group,
          ),
          (
            "{
  true;
  false;
  nil;
}",
            [
              true |> bool_prim |> of_expr,
              false |> bool_prim |> of_expr,
              nil_prim |> of_expr,
            ]
            |> of_closure,
          ),
          ("!true", true |> bool_prim |> of_not_op),
          (
            "true || false",
            (true |> bool_prim, false |> bool_prim) |> of_or_op,
          ),
          (
            "<Foo>
  bar
</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              ["bar" |> of_text |> as_lexeme],
            )
            |> of_tag
            |> as_lexeme
            |> of_jsx,
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_expression % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_jsx()"
    >: (
      () =>
        [
          ("<Foo />", ("Foo" |> of_public |> as_lexeme, [], []) |> of_tag),
          ("<></>", [] |> of_frag),
          (
            "<Foo #bar .fizz buzz />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                "bar" |> of_public |> as_lexeme |> of_jsx_id |> as_lexeme,
                ("fizz" |> of_public |> as_lexeme, None)
                |> of_jsx_class
                |> as_lexeme,
                ("buzz" |> of_public |> as_lexeme, None)
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag,
          ),
          (
            "<Foo>
  bar
</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              ["bar" |> of_text |> as_lexeme],
            )
            |> of_tag,
          ),
          (
            "<Foo>
  {1 + 5}
</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                (1 |> int_prim, 5 |> int_prim)
                |> of_add_op
                |> as_int
                |> of_inline_expr
                |> as_lexeme,
              ],
            )
            |> of_tag,
          ),
          (
            "<Foo>
  <Bar>
    fizzbuzz
  </Bar>
</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                (
                  "Bar" |> of_public |> as_lexeme,
                  [],
                  ["fizzbuzz" |> of_text |> as_lexeme],
                )
                |> of_tag
                |> as_lexeme
                |> of_node
                |> as_lexeme,
              ],
            )
            |> of_tag,
          ),
          (
            "<Foo>
  <Bar />
  {nil}
  Hello, World!
</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                ("Bar" |> of_public |> as_lexeme, [], [])
                |> of_tag
                |> as_lexeme
                |> of_node
                |> as_lexeme,
                nil_prim |> of_inline_expr |> as_lexeme,
                "Hello, World!" |> of_text |> as_lexeme,
              ],
            )
            |> of_tag,
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
              "fizz" |> of_public |> as_lexeme,
              Some("buzz" |> of_public |> as_lexeme |> of_id |> as_weak(0)),
            )
            |> of_prop,
          ),
          (
            "fizz=123",
            ("fizz" |> of_public |> as_lexeme, Some(123 |> int_prim))
            |> of_prop,
          ),
          (
            "fizz=(1 + 2)",
            (
              "fizz" |> of_public |> as_lexeme,
              Some((1 |> int_prim, 2 |> int_prim) |> of_add_op |> as_int),
            )
            |> of_prop,
          ),
          (
            "fizz=(-1)",
            (
              "fizz" |> of_public |> as_lexeme,
              Some(1 |> int_prim |> of_neg_op |> as_int),
            )
            |> of_prop,
          ),
          (
            "fizz=true",
            (
              "fizz" |> of_public |> as_lexeme,
              Some(
                true |> bool_prim |> of_group |> as_bool |> of_group |> as_bool,
              ),
            )
            |> of_prop,
          ),
          (
            "fizz=(1 + 2)",
            (
              "fizz" |> of_public |> as_lexeme,
              Some(
                (1 |> int_prim, 2 |> int_prim)
                |> of_add_op
                |> as_int
                |> of_group
                |> as_int,
              ),
            )
            |> of_prop,
          ),
          (
            "fizz={
  true;
  false;
}",
            (
              "fizz" |> of_public |> as_lexeme,
              Some(
                [true |> bool_prim |> of_expr, false |> bool_prim |> of_expr]
                |> of_closure
                |> as_bool,
              ),
            )
            |> of_prop,
          ),
          (
            "fizz=<Buzz />",
            (
              "fizz" |> of_public |> as_lexeme,
              Some(
                ("Buzz" |> of_public |> as_lexeme, [], [])
                |> of_tag
                |> as_lexeme
                |> of_jsx
                |> as_element,
              ),
            )
            |> of_prop,
          ),
          (
            "fizz=(<Buzz>
  <Foo />
</Buzz>)",
            (
              "fizz" |> of_public |> as_lexeme,
              Some(
                (
                  "Buzz" |> of_public |> as_lexeme,
                  [],
                  [
                    ("Foo" |> of_public |> as_lexeme, [], [])
                    |> of_tag
                    |> as_lexeme
                    |> of_node
                    |> as_lexeme,
                  ],
                )
                |> of_tag
                |> as_lexeme
                |> of_jsx
                |> as_element,
              ),
            )
            |> of_prop,
          ),
          ("buzz", ("buzz" |> of_public |> as_lexeme, None) |> of_prop),
          (
            ".fizz=true",
            ("fizz" |> of_public |> as_lexeme, Some(true |> bool_prim))
            |> of_jsx_class,
          ),
          (
            ".fizz",
            ("fizz" |> of_public |> as_lexeme, None) |> of_jsx_class,
          ),
          ("#bar", "bar" |> of_public |> as_lexeme |> of_jsx_id),
        ]
        |> List.map(Tuple.map_snd2(fmt_jsx_attr % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_statement()"
    >: (
      () =>
        [
          ("nil;", nil |> as_nil |> of_prim |> as_nil |> of_expr),
          (
            "let foo = nil;",
            (
              "foo" |> of_public |> as_lexeme,
              nil |> as_nil |> of_prim |> as_nil,
            )
            |> of_var,
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_statement % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_decl() - constant"
    >: (
      () =>
        [
          (
            "const foo = nil;\n",
            (
              "foo" |> of_public |> as_lexeme,
              nil |> as_nil |> of_prim |> as_nil |> of_const,
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_decl % Pretty.to_string))
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
                "DEF" |> of_public |> as_lexeme |> of_named_export,
                true |> bool_prim |> of_const,
              )
              |> of_decl,
              (
                "ABC" |> of_public |> as_lexeme |> of_named_export,
                123 |> int_prim |> of_const,
              )
              |> of_decl,
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
                "buzz" |> of_external,
                ["Fizz" |> of_public |> as_lexeme |> of_main_import],
              )
              |> of_import,
              (
                "bar" |> of_external,
                [
                  "bar" |> of_public |> as_lexeme |> of_main_import,
                  (
                    "Foo" |> of_public |> as_lexeme,
                    Some("foo" |> of_public |> as_lexeme),
                  )
                  |> of_named_import,
                  ("Bar" |> of_public |> as_lexeme, None) |> of_named_import,
                ],
              )
              |> of_import,
            ],
          ),
          (
            "import Fizz from \"buzz\";

import Foo from \"@/bar\";
",
            [
              (
                "bar" |> of_internal,
                ["Foo" |> of_public |> as_lexeme |> of_main_import],
              )
              |> of_import,
              (
                "buzz" |> of_external,
                ["Fizz" |> of_public |> as_lexeme |> of_main_import],
              )
              |> of_import,
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
                "bar" |> of_external,
                ["Foo" |> of_public |> as_lexeme |> of_main_import],
              )
              |> of_import,
            ],
          ),
          (
            "const ABC = 123;\n",
            [
              (
                "ABC" |> of_public |> as_lexeme |> of_named_export,
                123 |> int_prim |> of_const,
              )
              |> of_decl,
            ],
          ),
          (
            "import Foo from \"bar\";

const ABC = 123;
",
            [
              (
                "bar" |> of_external,
                ["Foo" |> of_public |> as_lexeme |> of_main_import],
              )
              |> of_import,
              (
                "ABC" |> of_public |> as_lexeme |> of_named_export,
                123 |> int_prim |> of_const,
              )
              |> of_decl,
            ],
          ),
        ]
        |> List.map(Tuple.map_snd2(Formatter.format % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
  ];
