open Kore;
open Util;
open Reference;

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
          ("123", 123 |> Int64.of_int |> RawUtil.int),
          ("9223372036854775807", Int64.max_int |> RawUtil.int),
          ("-9223372036854775808", Int64.min_int |> RawUtil.int),
          ("123.456", (123.456, 3) |> RawUtil.float),
        ]
        |> List.map(Tuple.map_snd2(fmt_num % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_prim()"
    >: (
      () =>
        [
          ("123", 123 |> Int64.of_int |> RawUtil.int |> RawUtil.num),
          ("true", true |> RawUtil.bool),
          ("false", false |> RawUtil.bool),
          ("nil", RawUtil.nil),
          ("\"foo bar\"", "foo bar" |> RawUtil.string),
        ]
        |> List.map(Tuple.map_snd2(fmt_prim % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_expression()"
    >: (
      () =>
        [
          ("nil", RawUtil.nil |> as_nil |> RawUtil.prim),
          ("fooBar", "fooBar" |> RawUtil.public |> as_lexeme |> RawUtil.id),
          ("123", 123 |> int_prim |> RawUtil.group),
          (
            "123",
            123 |> int_prim |> RawUtil.group |> as_int |> RawUtil.group,
          ),
          (
            "(123 + 456)",
            (123 |> int_prim, 456 |> int_prim)
            |> RawUtil.add_op
            |> as_int
            |> RawUtil.group,
          ),
          (
            "(123 + 456)",
            (123 |> int_prim, 456 |> int_prim)
            |> RawUtil.add_op
            |> as_int
            |> RawUtil.group
            |> as_int
            |> RawUtil.group,
          ),
          (
            "{
  true;
  false;
  nil;
}",
            [
              true |> bool_prim |> RawUtil.expr,
              false |> bool_prim |> RawUtil.expr,
              nil_prim |> RawUtil.expr,
            ]
            |> RawUtil.closure,
          ),
          ("!true", true |> bool_prim |> RawUtil.not_op),
          (
            "true || false",
            (true |> bool_prim, false |> bool_prim) |> RawUtil.or_op,
          ),
          (
            "<Foo>
  bar
</Foo>",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [],
              ["bar" |> as_lexeme |> RawUtil.text |> as_lexeme],
            )
            |> RawUtil.tag
            |> as_lexeme
            |> RawUtil.jsx,
          ),
        ]
        |> List.map(Tuple.map_snd2(fmt_expression % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_jsx()"
    >: (
      () =>
        [
          (
            "<Foo />",
            ("Foo" |> RawUtil.public |> as_lexeme, [], []) |> RawUtil.tag,
          ),
          ("<></>", [] |> RawUtil.frag),
          (
            "<Foo #bar .fizz buzz />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                "bar"
                |> RawUtil.public
                |> as_lexeme
                |> RawUtil.jsx_id
                |> as_lexeme,
                ("fizz" |> RawUtil.public |> as_lexeme, None)
                |> RawUtil.jsx_class
                |> as_lexeme,
                ("buzz" |> RawUtil.public |> as_lexeme, None)
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag,
          ),
          (
            "<Foo>
  bar
</Foo>",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [],
              ["bar" |> as_lexeme |> RawUtil.text |> as_lexeme],
            )
            |> RawUtil.tag,
          ),
          (
            "<Foo>
  {1 + 5}
</Foo>",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [],
              [
                (1 |> int_prim, 5 |> int_prim)
                |> RawUtil.add_op
                |> as_int
                |> RawUtil.inline_expr
                |> as_lexeme,
              ],
            )
            |> RawUtil.tag,
          ),
          (
            "<Foo>
  <Bar>
    fizzbuzz
  </Bar>
</Foo>",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [],
              [
                (
                  "Bar" |> RawUtil.public |> as_lexeme,
                  [],
                  ["fizzbuzz" |> as_lexeme |> RawUtil.text |> as_lexeme],
                )
                |> RawUtil.tag
                |> as_lexeme
                |> RawUtil.node
                |> as_lexeme,
              ],
            )
            |> RawUtil.tag,
          ),
          (
            "<Foo>
  <Bar />
  {nil}
  Hello, World!
</Foo>",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [],
              [
                ("Bar" |> RawUtil.public |> as_lexeme, [], [])
                |> RawUtil.tag
                |> as_lexeme
                |> RawUtil.node
                |> as_lexeme,
                nil_prim |> RawUtil.inline_expr |> as_lexeme,
                "Hello, World!" |> as_lexeme |> RawUtil.text |> as_lexeme,
              ],
            )
            |> RawUtil.tag,
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
              "fizz" |> RawUtil.public |> as_lexeme,
              Some(
                "buzz"
                |> RawUtil.public
                |> as_lexeme
                |> RawUtil.id
                |> as_weak(0),
              ),
            )
            |> RawUtil.prop,
          ),
          (
            "fizz=123",
            ("fizz" |> RawUtil.public |> as_lexeme, Some(123 |> int_prim))
            |> RawUtil.prop,
          ),
          (
            "fizz=(1 + 2)",
            (
              "fizz" |> RawUtil.public |> as_lexeme,
              Some(
                (1 |> int_prim, 2 |> int_prim) |> RawUtil.add_op |> as_int,
              ),
            )
            |> RawUtil.prop,
          ),
          (
            "fizz=(-1)",
            (
              "fizz" |> RawUtil.public |> as_lexeme,
              Some(1 |> int_prim |> RawUtil.neg_op |> as_int),
            )
            |> RawUtil.prop,
          ),
          (
            "fizz=true",
            (
              "fizz" |> RawUtil.public |> as_lexeme,
              Some(
                true
                |> bool_prim
                |> RawUtil.group
                |> as_bool
                |> RawUtil.group
                |> as_bool,
              ),
            )
            |> RawUtil.prop,
          ),
          (
            "fizz=(1 + 2)",
            (
              "fizz" |> RawUtil.public |> as_lexeme,
              Some(
                (1 |> int_prim, 2 |> int_prim)
                |> RawUtil.add_op
                |> as_int
                |> RawUtil.group
                |> as_int,
              ),
            )
            |> RawUtil.prop,
          ),
          (
            "fizz={
  true;
  false;
}",
            (
              "fizz" |> RawUtil.public |> as_lexeme,
              Some(
                [
                  true |> bool_prim |> RawUtil.expr,
                  false |> bool_prim |> RawUtil.expr,
                ]
                |> RawUtil.closure
                |> as_bool,
              ),
            )
            |> RawUtil.prop,
          ),
          (
            "fizz=<Buzz />",
            (
              "fizz" |> RawUtil.public |> as_lexeme,
              Some(
                ("Buzz" |> RawUtil.public |> as_lexeme, [], [])
                |> RawUtil.tag
                |> as_lexeme
                |> RawUtil.jsx
                |> as_element,
              ),
            )
            |> RawUtil.prop,
          ),
          (
            "fizz=(<Buzz>
  <Foo />
</Buzz>)",
            (
              "fizz" |> RawUtil.public |> as_lexeme,
              Some(
                (
                  "Buzz" |> RawUtil.public |> as_lexeme,
                  [],
                  [
                    ("Foo" |> RawUtil.public |> as_lexeme, [], [])
                    |> RawUtil.tag
                    |> as_lexeme
                    |> RawUtil.node
                    |> as_lexeme,
                  ],
                )
                |> RawUtil.tag
                |> as_lexeme
                |> RawUtil.jsx
                |> as_element,
              ),
            )
            |> RawUtil.prop,
          ),
          (
            "buzz",
            ("buzz" |> RawUtil.public |> as_lexeme, None) |> RawUtil.prop,
          ),
          (
            ".fizz=true",
            ("fizz" |> RawUtil.public |> as_lexeme, Some(true |> bool_prim))
            |> RawUtil.jsx_class,
          ),
          (
            ".fizz",
            ("fizz" |> RawUtil.public |> as_lexeme, None) |> RawUtil.jsx_class,
          ),
          ("#bar", "bar" |> RawUtil.public |> as_lexeme |> RawUtil.jsx_id),
        ]
        |> List.map(Tuple.map_snd2(fmt_jsx_attr % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_statement()"
    >: (
      () =>
        [
          (
            "nil;",
            RawUtil.nil |> as_nil |> RawUtil.prim |> as_nil |> RawUtil.expr,
          ),
          (
            "let foo = nil;",
            (
              "foo" |> RawUtil.public |> as_lexeme,
              RawUtil.nil |> as_nil |> RawUtil.prim |> as_nil,
            )
            |> RawUtil.var,
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
              "foo" |> RawUtil.public |> as_lexeme,
              RawUtil.nil |> as_nil |> RawUtil.prim |> as_nil |> RawUtil.const,
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
              "foo" |> RawUtil.public |> as_lexeme,
              (
                [
                  (
                    {
                      name: "bar" |> RawUtil.public |> as_lexeme,
                      default: None,
                    },
                    Type.K_Strong(K_Integer),
                  ),
                  (
                    {
                      name: "fizz" |> RawUtil.public |> as_lexeme,
                      default: Some(3 |> int_prim),
                    },
                    Type.K_Strong(K_Integer),
                  ),
                ],
                (
                  "bar" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                  "fizz" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                )
                |> RawUtil.add_op
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
              "buzz" |> RawUtil.public |> as_lexeme,
              (
                [],
                [
                  ("zip" |> RawUtil.public |> as_lexeme, 3 |> int_prim)
                  |> RawUtil.var,
                  ("zap" |> RawUtil.public |> as_lexeme, 4 |> int_prim)
                  |> RawUtil.var,
                  (
                    "zip" |> RawUtil.public |> as_lexeme |> RawUtil.id |> as_int,
                    "zap"
                    |> RawUtil.public
                    |> as_lexeme
                    |> RawUtil.id
                    |> as_int,
                  )
                  |> RawUtil.mult_op
                  |> as_int
                  |> RawUtil.expr,
                ]
                |> RawUtil.closure
                |> as_int,
              )
              |> RawUtil.func,
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
                "DEF" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
                true |> bool_prim |> RawUtil.const,
              )
              |> RawUtil.decl,
              (
                "ABC" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
                123 |> int_prim |> RawUtil.const,
              )
              |> RawUtil.decl,
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
                "buzz" |> RawUtil.external,
                ["Fizz" |> RawUtil.public |> as_lexeme |> RawUtil.main_import],
              )
              |> RawUtil.import,
              (
                "bar" |> RawUtil.external,
                [
                  "bar" |> RawUtil.public |> as_lexeme |> RawUtil.main_import,
                  (
                    "Foo" |> RawUtil.public |> as_lexeme,
                    Some("foo" |> RawUtil.public |> as_lexeme),
                  )
                  |> RawUtil.named_import,
                  ("Bar" |> RawUtil.public |> as_lexeme, None)
                  |> RawUtil.named_import,
                ],
              )
              |> RawUtil.import,
            ],
          ),
          (
            "import Fizz from \"buzz\";

import Foo from \"@/bar\";
",
            [
              (
                "bar" |> RawUtil.internal,
                ["Foo" |> RawUtil.public |> as_lexeme |> RawUtil.main_import],
              )
              |> RawUtil.import,
              (
                "buzz" |> RawUtil.external,
                ["Fizz" |> RawUtil.public |> as_lexeme |> RawUtil.main_import],
              )
              |> RawUtil.import,
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
                "bar" |> RawUtil.external,
                ["Foo" |> RawUtil.public |> as_lexeme |> RawUtil.main_import],
              )
              |> RawUtil.import,
            ],
          ),
          (
            "const ABC = 123;\n",
            [
              (
                "ABC" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
                123 |> int_prim |> RawUtil.const,
              )
              |> RawUtil.decl,
            ],
          ),
          (
            "import Foo from \"bar\";

const ABC = 123;
",
            [
              (
                "bar" |> RawUtil.external,
                ["Foo" |> RawUtil.public |> as_lexeme |> RawUtil.main_import],
              )
              |> RawUtil.import,
              (
                "ABC" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
                123 |> int_prim |> RawUtil.const,
              )
              |> RawUtil.decl,
            ],
          ),
        ]
        |> List.map(Tuple.map_snd2(Formatter.format % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
  ];
