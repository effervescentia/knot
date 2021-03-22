open Kore;
open AST;
open Util;

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
    >: (
      () =>
        [("fooBar", Public("fooBar")), ("_fooBar", Private("fooBar"))]
        |> List.map(Tuple.map_snd2(fmt_id % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_ns()"
    >: (
      () =>
        [("@/fooBar", Internal("fooBar")), ("fooBar", External("fooBar"))]
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
    "fmt_expr()"
    >: (
      () =>
        [
          ("nil", nil |> as_nil |> of_prim),
          ("fooBar", "fooBar" |> of_public |> as_lexeme |> of_id),
          (
            "(123)",
            123
            |> Int64.of_int
            |> of_int
            |> of_num
            |> as_int
            |> of_prim
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
              true |> of_bool |> as_bool |> of_prim |> as_bool |> of_expr,
              false |> of_bool |> as_bool |> of_prim |> as_bool |> of_expr,
              nil |> as_nil |> of_prim |> as_nil |> of_expr,
            ]
            |> of_closure,
          ),
          (
            "!true",
            true |> of_bool |> as_bool |> of_prim |> as_bool |> of_not_op,
          ),
          (
            "true || false",
            (
              true |> of_bool |> as_bool |> of_prim |> as_bool,
              false |> of_bool |> as_bool |> of_prim |> as_bool,
            )
            |> of_or_op,
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
        |> List.map(Tuple.map_snd2(fmt_expr % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_jsx()"
    >: (
      () =>
        [
          ("<Foo />", ("Foo" |> of_public |> as_lexeme, [], []) |> of_tag),
          ("<></>", [] |> of_frag),
          (
            "<Foo fizz=buzz />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  Some(
                    "buzz" |> of_public |> as_lexeme |> of_id |> as_unknown,
                  ),
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag,
          ),
          (
            "<Foo .fizz=true />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  Some(true |> bool_prim |> as_bool),
                )
                |> of_jsx_class
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag,
          ),
          (
            "<Foo #bar />",
            (
              "Foo" |> of_public |> as_lexeme,
              ["bar" |> of_public |> as_lexeme |> of_jsx_id |> as_lexeme],
              [],
            )
            |> of_tag,
          ),
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
                nil_prim |> as_nil |> of_inline_expr |> as_lexeme,
                "Hello, World!" |> of_text |> as_lexeme,
              ],
            )
            |> of_tag,
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_jsx % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_stmt()"
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
        |> List.map(Tuple.map_snd2(fmt_stmt % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_decl() - constant"
    >: (
      () =>
        [
          (
            "const foo = nil;",
            (
              "foo" |> of_public |> as_lexeme,
              nil |> as_nil |> of_prim |> as_nil |> of_const,
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_decl % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_mod_stmt()"
    >: (
      () =>
        [
          (
            "const foo = nil;\n",
            (
              "foo" |> of_public |> as_lexeme,
              nil |> as_nil |> of_prim |> as_nil |> of_const,
            )
            |> of_decl,
          ),
          (
            "import Foo from \"bar\";\n",
            ("bar" |> of_external, "Foo") |> of_import,
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_mod_stmt % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "format()"
    >: (
      () =>
        [
          ("", [] |> Formatter.format),
          (
            "import Foo from \"bar\";
const ABC = 123;
",
            [
              ("bar" |> of_external, "Foo") |> of_import,
              ("ABC" |> of_public |> as_lexeme, int_prim(123) |> of_const)
              |> of_decl,
            ]
            |> Formatter.format,
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
