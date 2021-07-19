open Kore;
open Util;

module Expression = Grammar.Expression;
module JSX = Grammar.JSX;

module Assert =
  Assert.Make({
    open AST.Raw;

    type t = jsx_t;

    let parser = ctx =>
      JSX.parser(ctx, (Expression.expr_4, Expression.parser)) |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              fst
              % Debug.print_jsx
              % Cow.Xml.to_string
              % Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.JSX"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse tag"
    >: (
      () =>
        [
          (
            "<Foo></Foo>",
            ("Foo" |> RawUtil.public |> as_lexeme, [], [])
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            " < Foo > < / Foo > ",
            ("Foo" |> RawUtil.public |> as_lexeme, [], [])
            |> RawUtil.tag
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse self-closing tag"
    >: (
      () =>
        [
          (
            "<Foo/>",
            ("Foo" |> RawUtil.public |> as_lexeme, [], [])
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            " < Foo / > ",
            ("Foo" |> RawUtil.public |> as_lexeme, [], [])
            |> RawUtil.tag
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse fragment"
    >: (
      () =>
        [
          ("<></>", [] |> RawUtil.frag |> as_lexeme),
          (
            "<><Bar /></>",
            [
              ("Bar" |> RawUtil.public |> as_lexeme, [], [])
              |> jsx_node
              |> as_lexeme,
            ]
            |> RawUtil.frag
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse attributes"
    >: (
      () =>
        [
          (
            "<Foo fizz=buzz />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                (
                  "fizz" |> RawUtil.public |> as_lexeme,
                  "buzz"
                  |> RawUtil.public
                  |> as_lexeme
                  |> RawUtil.id
                  |> as_bool
                  |> Option.some,
                )
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=\"buzz\" />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                (
                  "fizz" |> RawUtil.public |> as_lexeme,
                  "buzz" |> string_prim |> Option.some,
                )
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz={ buzz; } />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                (
                  "fizz" |> RawUtil.public |> as_lexeme,
                  [
                    "buzz"
                    |> RawUtil.public
                    |> as_lexeme
                    |> RawUtil.id
                    |> as_bool
                    |> RawUtil.expr,
                  ]
                  |> RawUtil.closure
                  |> as_bool
                  |> Option.some,
                )
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=1 + 2 />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                (
                  "fizz" |> RawUtil.public |> as_lexeme,
                  (1 |> int_prim, 2 |> int_prim)
                  |> RawUtil.add_op
                  |> as_int
                  |> Option.some,
                )
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=(1 > 2) />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                (
                  "fizz" |> RawUtil.public |> as_lexeme,
                  (1 |> int_prim, 2 |> int_prim)
                  |> RawUtil.gt_op
                  |> as_bool
                  |> Option.some,
                )
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=(true) />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                (
                  "fizz" |> RawUtil.public |> as_lexeme,
                  bool_prim(true) |> Option.some,
                )
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=-3 />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                (
                  "fizz" |> RawUtil.public |> as_lexeme,
                  3 |> int_prim |> RawUtil.neg_op |> as_int |> Option.some,
                )
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=<buzz /> />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                (
                  "fizz" |> RawUtil.public |> as_lexeme,
                  ("buzz" |> RawUtil.public |> as_lexeme, [], [])
                  |> jsx_tag
                  |> as_element
                  |> Option.some,
                )
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                ("fizz" |> RawUtil.public |> as_lexeme, None)
                |> RawUtil.prop
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo .fizz />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                ("fizz" |> RawUtil.public |> as_lexeme, None)
                |> RawUtil.jsx_class
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo #fizz />",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [
                "fizz"
                |> RawUtil.public
                |> as_lexeme
                |> RawUtil.jsx_id
                |> as_lexeme,
              ],
              [],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many(
             ~scope=
               to_scope([
                 ("fizz", K_Strong(K_Integer)),
                 ("buzz", K_Strong(K_Boolean)),
               ]),
           )
    ),
    "parse single child"
    >: (
      () =>
        [
          (
            "<Foo><Bar /></Foo>",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [],
              [
                ("Bar" |> RawUtil.public |> as_lexeme, [], [])
                |> jsx_node
                |> as_lexeme,
              ],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo>{1 + 2}</Foo>",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [],
              [
                (1 |> int_prim, 2 |> int_prim)
                |> RawUtil.add_op
                |> as_int
                |> RawUtil.inline_expr
                |> as_lexeme,
              ],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo>{<Bar />}</Foo>",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [],
              [
                ("Bar" |> RawUtil.public |> as_lexeme, [], [])
                |> jsx_tag
                |> as_element
                |> RawUtil.inline_expr
                |> as_lexeme,
              ],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
          (
            "<Foo> bar \"or\" 123 </Foo>",
            (
              "Foo" |> RawUtil.public |> as_lexeme,
              [],
              ["bar \"or\" 123" |> as_lexeme |> RawUtil.text |> as_lexeme],
            )
            |> RawUtil.tag
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse complex"
    >: (
      () =>
        try(
          [
            (
              "<Foo bar=4><Bar /></Foo>",
              (
                "Foo" |> RawUtil.public |> as_lexeme,
                [
                  (
                    "bar" |> RawUtil.public |> as_lexeme,
                    4 |> int_prim |> Option.some,
                  )
                  |> RawUtil.prop
                  |> as_lexeme,
                ],
                [
                  ("Bar" |> RawUtil.public |> as_lexeme, [], [])
                  |> jsx_node
                  |> as_lexeme,
                ],
              )
              |> RawUtil.tag
              |> as_lexeme,
            ),
            (
              "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
              (
                "Foo" |> RawUtil.public |> as_lexeme,
                [],
                [
                  "bar" |> as_lexeme |> RawUtil.text |> as_lexeme,
                  (1 |> int_prim, 2 |> int_prim)
                  |> RawUtil.add_op
                  |> as_int
                  |> RawUtil.inline_expr
                  |> as_lexeme,
                  ("Bar" |> RawUtil.public |> as_lexeme, [], [])
                  |> jsx_node
                  |> as_lexeme,
                  "fizz" |> string_prim |> RawUtil.inline_expr |> as_lexeme,
                  "buzz" |> as_lexeme |> RawUtil.text |> as_lexeme,
                ],
              )
              |> RawUtil.tag
              |> as_lexeme,
            ),
            (
              "<Foo bar=fizz .buzz />",
              (
                "Foo" |> RawUtil.public |> as_lexeme,
                [
                  (
                    "bar" |> RawUtil.public |> as_lexeme,
                    "fizz"
                    |> RawUtil.public
                    |> as_lexeme
                    |> RawUtil.id
                    |> as_bool
                    |> Option.some,
                  )
                  |> RawUtil.prop
                  |> as_lexeme,
                  ("buzz" |> RawUtil.public |> as_lexeme, None)
                  |> RawUtil.jsx_class
                  |> as_lexeme,
                ],
                [],
              )
              |> RawUtil.tag
              |> as_lexeme,
            ),
          ]
          |> Assert.parse_many(
               ~scope=to_scope([("fizz", K_Strong(K_Boolean))]),
             )
        ) {
        | CompileError(x) =>
          x
          |> List.map(Knot.Error._compile_err_to_string)
          |> List.intersperse(", ")
          |> List.fold_left((++), "")
          |> Test.Assert.string("%s")
        }
    ),
  ];
