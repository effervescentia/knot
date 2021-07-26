open Kore;
open Util.RawUtil;
open AST.Raw;

module Expression = Grammar.Expression;
module JSX = Grammar.JSX;

module Assert =
  Assert.Make({
    type t = jsx_t;

    let parser = ((_, _, ctx)) =>
      JSX.parser(ctx, (Expression.expr_4, Expression.parser)) |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              fst
              % Debug.print_jsx
              % Pretty.to_string
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
            ("Foo" |> of_public |> as_lexeme, [], []) |> of_tag |> as_lexeme,
          ),
          (
            " < Foo > < / Foo > ",
            ("Foo" |> of_public |> as_lexeme, [], []) |> of_tag |> as_lexeme,
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
            ("Foo" |> of_public |> as_lexeme, [], []) |> of_tag |> as_lexeme,
          ),
          (
            " < Foo / > ",
            ("Foo" |> of_public |> as_lexeme, [], []) |> of_tag |> as_lexeme,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse fragment"
    >: (
      () =>
        [
          ("<></>", [] |> of_frag |> as_lexeme),
          (
            "<><Bar /></>",
            [
              ("Bar" |> of_public |> as_lexeme, [], [])
              |> jsx_node
              |> as_lexeme,
            ]
            |> of_frag
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
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  "buzz"
                  |> of_public
                  |> as_lexeme
                  |> of_id
                  |> as_lexeme
                  |> Option.some,
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=\"buzz\" />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  "buzz" |> string_prim |> Option.some,
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz={ buzz; } />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  [
                    "buzz"
                    |> of_public
                    |> as_lexeme
                    |> of_id
                    |> as_lexeme
                    |> of_expr,
                  ]
                  |> of_closure
                  |> as_lexeme
                  |> Option.some,
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=1 + 2 />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  (1 |> int_prim, 2 |> int_prim)
                  |> of_add_op
                  |> as_lexeme
                  |> Option.some,
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=(1 > 2) />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  (1 |> int_prim, 2 |> int_prim)
                  |> of_gt_op
                  |> as_lexeme
                  |> Option.some,
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=(true) />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  bool_prim(true) |> Option.some,
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=-3 />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  3 |> int_prim |> of_neg_op |> as_lexeme |> Option.some,
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz=<buzz /> />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  ("buzz" |> of_public |> as_lexeme, [], [])
                  |> jsx_tag
                  |> as_lexeme
                  |> Option.some,
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo fizz />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                ("fizz" |> of_public |> as_lexeme, None)
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo .fizz />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                ("fizz" |> of_public |> as_lexeme, None)
                |> of_jsx_class
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo #fizz />",
            (
              "Foo" |> of_public |> as_lexeme,
              ["fizz" |> of_public |> as_lexeme |> of_jsx_id |> as_lexeme],
              [],
            )
            |> of_tag
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many(
             ~cls_context=
               scope_to_closure([
                 ("fizz", Strong(`Integer)),
                 ("buzz", Strong(`Boolean)),
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
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                ("Bar" |> of_public |> as_lexeme, [], [])
                |> jsx_node
                |> as_lexeme,
              ],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo>{1 + 2}</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                (1 |> int_prim, 2 |> int_prim)
                |> of_add_op
                |> as_lexeme
                |> of_inline_expr
                |> as_lexeme,
              ],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo>{<Bar />}</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                ("Bar" |> of_public |> as_lexeme, [], [])
                |> jsx_tag
                |> as_lexeme
                |> of_inline_expr
                |> as_lexeme,
              ],
            )
            |> of_tag
            |> as_lexeme,
          ),
          (
            "<Foo> bar \"or\" 123 </Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              ["bar \"or\" 123" |> as_lexeme |> of_text |> as_lexeme],
            )
            |> of_tag
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
                "Foo" |> of_public |> as_lexeme,
                [
                  (
                    "bar" |> of_public |> as_lexeme,
                    4 |> int_prim |> Option.some,
                  )
                  |> of_prop
                  |> as_lexeme,
                ],
                [
                  ("Bar" |> of_public |> as_lexeme, [], [])
                  |> jsx_node
                  |> as_lexeme,
                ],
              )
              |> of_tag
              |> as_lexeme,
            ),
            (
              "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
              (
                "Foo" |> of_public |> as_lexeme,
                [],
                [
                  "bar" |> as_lexeme |> of_text |> as_lexeme,
                  (1 |> int_prim, 2 |> int_prim)
                  |> of_add_op
                  |> as_lexeme
                  |> of_inline_expr
                  |> as_lexeme,
                  ("Bar" |> of_public |> as_lexeme, [], [])
                  |> jsx_node
                  |> as_lexeme,
                  "fizz" |> string_prim |> of_inline_expr |> as_lexeme,
                  "buzz" |> as_lexeme |> of_text |> as_lexeme,
                ],
              )
              |> of_tag
              |> as_lexeme,
            ),
            (
              "<Foo bar=fizz .buzz />",
              (
                "Foo" |> of_public |> as_lexeme,
                [
                  (
                    "bar" |> of_public |> as_lexeme,
                    "fizz"
                    |> of_public
                    |> as_lexeme
                    |> of_id
                    |> as_lexeme
                    |> Option.some,
                  )
                  |> of_prop
                  |> as_lexeme,
                  ("buzz" |> of_public |> as_lexeme, None)
                  |> of_jsx_class
                  |> as_lexeme,
                ],
                [],
              )
              |> of_tag
              |> as_lexeme,
            ),
          ]
          |> Assert.parse_many(
               ~cls_context=scope_to_closure([("fizz", Strong(`Boolean))]),
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
