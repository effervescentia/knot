open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Formatter = Grammar.Formatter;

let __int_const = ("ABC" |> of_public, 123 |> int_prim |> of_const);
let __bool_const = ("DEF" |> of_public, true |> bool_prim |> of_const);

let __inline_function = (
  "foo" |> of_public,
  (
    [
      {name: "bar" |> of_public |> as_raw_node, default: None, type_: None}
      |> as_int,
      {
        name: "fizz" |> of_public |> as_raw_node,
        default: Some(3 |> int_prim),
        type_: None,
      }
      |> as_int,
    ],
    (
      "bar" |> of_public |> as_int |> of_id |> as_int,
      "fizz" |> of_public |> as_int |> of_id |> as_int,
    )
    |> of_add_op
    |> as_int,
  )
  |> of_func,
);

let __multiline_function = (
  "buzz" |> of_public,
  (
    [],
    [
      ("zip" |> of_public |> as_raw_node, 3 |> int_prim) |> of_var |> as_nil,
      ("zap" |> of_public |> as_raw_node, 4 |> int_prim) |> of_var |> as_nil,
      (
        "zip" |> of_public |> as_int |> of_id |> as_int,
        "zap" |> of_public |> as_int |> of_id |> as_int,
      )
      |> of_mult_op
      |> as_int
      |> of_expr
      |> as_int,
    ]
    |> of_closure
    |> as_int,
  )
  |> of_func,
);

let __int_const_stmt =
  (
    "ABC" |> of_public |> as_raw_node |> of_named_export,
    123 |> int_prim |> of_const |> as_int,
  )
  |> of_decl;

let __import_stmt =
  (
    "bar" |> of_external,
    ["Foo" |> of_public |> as_raw_node |> of_main_import |> as_raw_node],
  )
  |> of_import;

let _main_import = (name, f) =>
  (
    name |> f,
    [
      name
      |> String.capitalize_ascii
      |> of_public
      |> as_raw_node
      |> of_main_import
      |> as_raw_node,
    ],
  )
  |> of_import;

let suite =
  "Grammar.Formatter"
  >::: [
    "pp_binary_op()"
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
        |> List.map(Tuple.map_snd2(~@pp_binary_op))
        |> Assert.(test_many(string))
    ),
    "pp_unary_op()"
    >: (
      () =>
        [("!", Not), ("+", Positive), ("-", Negative)]
        |> List.map(Tuple.map_snd2(~@pp_unary_op))
        |> Assert.(test_many(string))
    ),
    "pp_ns()"
    >: (
      () =>
        Namespace.[
          ("\"@/fooBar\"", Internal("fooBar")),
          ("\"fooBar\"", External("fooBar")),
        ]
        |> List.map(Tuple.map_snd2(~@pp_ns))
        |> Assert.(test_many(string))
    ),
    "pp_num()"
    >: (
      () =>
        [
          ("123", 123L |> of_int),
          ("9223372036854775807", Int64.max_int |> of_int),
          ("-9223372036854775808", Int64.min_int |> of_int),
          ("123.456", (123.456, 3) |> of_float),
        ]
        |> List.map(Tuple.map_snd2(~@pp_num))
        |> Assert.(test_many(string))
    ),
    "pp_prim()"
    >: (
      () =>
        [
          ("123", 123L |> of_int |> of_num),
          ("true", true |> of_bool),
          ("false", false |> of_bool),
          ("nil", nil),
          ("\"foo bar\"", "foo bar" |> of_string),
        ]
        |> List.map(Tuple.map_snd2(~@pp_prim))
        |> Assert.(test_many(string))
    ),
    "pp_expression()"
    >: (
      () =>
        [
          ("nil", nil |> as_nil |> of_prim),
          ("fooBar", "fooBar" |> of_public |> as_generic(0) |> of_id),
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
              true |> bool_prim |> of_expr |> as_bool,
              false |> bool_prim |> of_expr |> as_bool,
              nil_prim |> of_expr |> as_nil,
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
              "Foo" |> of_public |> as_raw_node,
              [],
              ["bar" |> as_string |> of_text |> as_string],
            )
            |> of_tag
            |> as_element
            |> of_jsx,
          ),
        ]
        |> List.map(Tuple.map_snd2(~@Fmt.root(pp_expression)))
        |> Assert.(test_many(string))
    ),
    "pp_jsx()"
    >: (
      () =>
        [
          ("<Foo />", ("Foo" |> of_public |> as_raw_node, [], []) |> of_tag),
          ("<></>", [] |> of_frag),
          (
            "<Foo #bar .fizz buzz />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                "bar" |> of_public |> as_raw_node |> of_jsx_id |> as_string,
                ("fizz" |> of_public |> as_raw_node, None)
                |> of_jsx_class
                |> as_string,
                ("buzz" |> of_public |> as_raw_node, None)
                |> of_prop
                |> as_generic(0),
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
              "Foo" |> of_public |> as_raw_node,
              [],
              ["bar" |> as_string |> of_text |> as_string],
            )
            |> of_tag,
          ),
          (
            "<Foo>
  {1 + 5}
</Foo>",
            (
              "Foo" |> of_public |> as_raw_node,
              [],
              [
                (1 |> int_prim, 5 |> int_prim)
                |> of_add_op
                |> as_int
                |> of_inline_expr
                |> as_element,
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
              "Foo" |> of_public |> as_raw_node,
              [],
              [
                (
                  "Bar" |> of_public |> as_raw_node,
                  [],
                  ["fizzbuzz" |> as_string |> of_text |> as_string],
                )
                |> of_tag
                |> as_element
                |> of_node
                |> as_element,
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
              "Foo" |> of_public |> as_raw_node,
              [],
              [
                ("Bar" |> of_public |> as_raw_node, [], [])
                |> of_tag
                |> as_element
                |> of_node
                |> as_element,
                nil_prim |> of_inline_expr |> as_nil,
                "Hello, World!" |> as_string |> of_text |> as_string,
              ],
            )
            |> of_tag,
          ),
        ]
        |> List.map(Tuple.map_snd2(~@Fmt.root(pp_jsx)))
        |> Assert.(test_many(string))
    ),
    "pp_jsx_attr()"
    >: (
      () =>
        [
          (
            "fizz=buzz",
            (
              "fizz" |> of_public |> as_raw_node,
              Some(
                "buzz"
                |> of_public
                |> as_generic(0)
                |> of_id
                |> as_generic(0),
              ),
            )
            |> of_prop,
          ),
          (
            "fizz=123",
            ("fizz" |> of_public |> as_raw_node, Some(123 |> int_prim))
            |> of_prop,
          ),
          (
            "fizz=(1 + 2)",
            (
              "fizz" |> of_public |> as_raw_node,
              Some((1 |> int_prim, 2 |> int_prim) |> of_add_op |> as_int),
            )
            |> of_prop,
          ),
          (
            "fizz=(-1)",
            (
              "fizz" |> of_public |> as_raw_node,
              Some(1 |> int_prim |> of_neg_op |> as_int),
            )
            |> of_prop,
          ),
          (
            "fizz=true",
            (
              "fizz" |> of_public |> as_raw_node,
              Some(
                true |> bool_prim |> of_group |> as_bool |> of_group |> as_bool,
              ),
            )
            |> of_prop,
          ),
          (
            "fizz=(1 + 2)",
            (
              "fizz" |> of_public |> as_raw_node,
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
              "fizz" |> of_public |> as_raw_node,
              Some(
                [
                  true |> bool_prim |> of_expr |> as_bool,
                  false |> bool_prim |> of_expr |> as_bool,
                ]
                |> of_closure
                |> as_bool,
              ),
            )
            |> of_prop,
          ),
          (
            "fizz=<Buzz />",
            (
              "fizz" |> of_public |> as_raw_node,
              Some(
                ("Buzz" |> of_public |> as_raw_node, [], [])
                |> of_tag
                |> as_element
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
              "fizz" |> of_public |> as_raw_node,
              Some(
                (
                  "Buzz" |> of_public |> as_raw_node,
                  [],
                  [
                    ("Foo" |> of_public |> as_raw_node, [], [])
                    |> of_tag
                    |> as_element
                    |> of_node
                    |> as_element,
                  ],
                )
                |> of_tag
                |> as_element
                |> of_jsx
                |> as_element,
              ),
            )
            |> of_prop,
          ),
          ("buzz", ("buzz" |> of_public |> as_raw_node, None) |> of_prop),
          (
            ".fizz=true",
            ("fizz" |> of_public |> as_raw_node, Some(true |> bool_prim))
            |> of_jsx_class,
          ),
          (
            ".fizz",
            ("fizz" |> of_public |> as_raw_node, None) |> of_jsx_class,
          ),
          ("#bar", "bar" |> of_public |> as_raw_node |> of_jsx_id),
        ]
        |> List.map(Tuple.map_snd2(~@Fmt.root(pp_jsx_attr)))
        |> Assert.(test_many(string))
    ),
    "pp_statement()"
    >: (
      () =>
        [
          ("nil;", nil |> as_nil |> of_prim |> as_nil |> of_expr),
          (
            "let foo = nil;",
            (
              "foo" |> of_public |> as_raw_node,
              nil |> as_nil |> of_prim |> as_nil,
            )
            |> of_var,
          ),
        ]
        |> List.map(Tuple.map_snd2(~@pp_statement))
        |> Assert.(test_many(string))
    ),
    "pp_declaration() - constant"
    >: (
      () =>
        [
          (
            "const foo = nil;",
            (
              "foo" |> of_public,
              nil |> as_nil |> of_prim |> as_nil |> of_const,
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(~@pp_declaration))
        |> Assert.(test_many(string))
    ),
    "pp_declaration() - function"
    >: (
      () =>
        [
          ("func foo(bar, fizz = 3) -> bar + fizz;", __inline_function),
          (
            "func buzz -> {
  let zip = 3;
  let zap = 4;
  zip * zap;
}",
            __multiline_function,
          ),
        ]
        |> List.map(Tuple.map_snd2(~@pp_declaration))
        |> Assert.(test_many(string))
    ),
    "pp_declaration_list()"
    >: (
      () =>
        [
          ("", []),
          ("const ABC = 123;", [__int_const]),
          (
            "const DEF = true;
const ABC = 123;",
            [__bool_const, __int_const],
          ),
          (
            "const DEF = true;

func foo(bar, fizz = 3) -> bar + fizz;

const ABC = 123;",
            [__bool_const, __inline_function, __int_const],
          ),
          (
            "func foo(bar, fizz = 3) -> bar + fizz;

const DEF = true;
const ABC = 123;",
            [__inline_function, __bool_const, __int_const],
          ),
          (
            "const DEF = true;
const ABC = 123;

func foo(bar, fizz = 3) -> bar + fizz;",
            [__bool_const, __int_const, __inline_function],
          ),
        ]
        |> List.map(Tuple.map_snd2(~@Fmt.root(pp_declaration_list)))
        |> Assert.(test_many(string))
    ),
    "pp_import()"
    >: (
      () =>
        [
          (
            "import Fizz from \"buzz\";",
            ("buzz" |> of_external, Some("Fizz" |> of_public), []),
          ),
          (
            "import { Foo as foo, Bar } from \"buzz\";",
            (
              "buzz" |> of_external,
              None,
              [
                ("Foo" |> of_public, Some("foo" |> of_public |> as_raw_node)),
                ("Bar" |> of_public, None),
              ],
            ),
          ),
          (
            "import Fizz, { Foo as foo, Bar } from \"buzz\";",
            (
              "buzz" |> of_external,
              Some("Fizz" |> of_public),
              [
                ("Foo" |> of_public, Some("foo" |> of_public |> as_raw_node)),
                ("Bar" |> of_public, None),
              ],
            ),
          ),
          (
            "import {
  Sit,
  et,
  dolore,
  est,
  aliqua,
  in,
  fugiat,
  duis,
  officia,
  pariatur,
  fugiat,
  mollit,
  eiusmod,
  pariatur,
  magna,
} from \"buzz\";",
            (
              "buzz" |> of_external,
              None,
              [
                "Sit",
                "et",
                "dolore",
                "est",
                "aliqua",
                "in",
                "fugiat",
                "duis",
                "officia",
                "pariatur",
                "fugiat",
                "mollit",
                "eiusmod",
                "pariatur",
                "magna",
              ]
              |> List.map(word => (word |> of_public, None)),
            ),
          ),
          (
            "import Foo, {
  Sit,
  et,
  dolore,
  est,
  aliqua,
  in,
  fugiat,
  duis,
  officia,
  pariatur,
  fugiat,
  mollit,
  eiusmod,
  pariatur,
  magna,
} from \"buzz\";",
            (
              "buzz" |> of_external,
              Some("Foo" |> of_public),
              [
                "Sit",
                "et",
                "dolore",
                "est",
                "aliqua",
                "in",
                "fugiat",
                "duis",
                "officia",
                "pariatur",
                "fugiat",
                "mollit",
                "eiusmod",
                "pariatur",
                "magna",
              ]
              |> List.map(word => (word |> of_public, None)),
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(~@Fmt.root(pp_import)))
        |> Assert.(test_many(string))
    ),
    "pp_all_imports()"
    >: (
      () => {
        let _main_import = (name, f) => (
          name |> f,
          Some(name |> String.capitalize_ascii |> of_public),
          [],
        );

        [
          (
            "import Foo from \"foo\";
import Bar from \"bar\";",
            (
              [],
              [
                _main_import("foo", of_external),
                _main_import("bar", of_external),
              ],
            ),
          ),
          (
            "import Foo from \"@/foo\";
import Bar from \"@/bar\";",
            (
              [
                _main_import("foo", of_internal),
                _main_import("bar", of_internal),
              ],
              [],
            ),
          ),
          (
            "import Bar from \"bar\";

import Foo from \"@/foo\";",
            (
              [_main_import("foo", of_internal)],
              [_main_import("bar", of_external)],
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(~@Fmt.root(pp_all_imports)))
        |> Assert.(test_many(string));
      }
    ),
    "format()"
    >: (
      () =>
        [
          ("\n", []),
          ("import Foo from \"bar\";\n", [__import_stmt]),
          ("const ABC = 123;\n", [__int_const_stmt]),
          (
            "import Foo from \"bar\";

const ABC = 123;\n",
            [__import_stmt, __int_const_stmt],
          ),
        ]
        |> List.map(
             Tuple.map_snd2(List.map(as_raw_node) % ~@Formatter.format),
           )
        |> Assert.(test_many(string))
    ),
    "format() - sort import statements"
    >: (
      () =>
        [
          (
            "import Bar from \"bar\";
import Foo from \"foo\";

import Buzz from \"@/buzz\";
import Fizz from \"@/fizz\";\n",
            [
              _main_import("buzz", of_internal),
              _main_import("bar", of_external),
              _main_import("fizz", of_internal),
              _main_import("foo", of_external),
            ],
          ),
        ]
        |> List.map(
             Tuple.map_snd2(List.map(as_raw_node) % ~@Formatter.format),
           )
        |> Assert.(test_many(string))
    ),
    "format() - sort named imports"
    >: (
      () =>
        [
          (
            "import { a, b, c, d } from \"foo\";\n",
            [
              (
                "foo" |> of_external,
                [
                  ("d" |> of_public |> as_raw_node, None)
                  |> of_named_import
                  |> as_raw_node,
                  ("c" |> of_public |> as_raw_node, None)
                  |> of_named_import
                  |> as_raw_node,
                  ("b" |> of_public |> as_raw_node, None)
                  |> of_named_import
                  |> as_raw_node,
                  ("a" |> of_public |> as_raw_node, None)
                  |> of_named_import
                  |> as_raw_node,
                ],
              )
              |> of_import,
            ],
          ),
        ]
        |> List.map(
             Tuple.map_snd2(List.map(as_raw_node) % ~@Formatter.format),
           )
        |> Assert.(test_many(string))
    ),
  ];
