open Kore;
open Util.RawUtil;
open AST.Raw;

module Expression = Grammar.Expression;
module JSX = Grammar.JSX;

module Assert =
  Assert.Make({
    type t = jsx_t;

    let parser = ((_, ctx)) =>
      JSX.parser(ctx, (Expression.expr_4, Expression.parser)) |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Node.Raw.value
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
            ("Foo" |> of_public |> as_raw_node, [], [])
            |> of_tag
            |> as_raw_node,
          ),
          (
            " < Foo > < / Foo > ",
            ("Foo" |> of_public |> as_raw_node, [], [])
            |> of_tag
            |> as_raw_node,
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
            ("Foo" |> of_public |> as_raw_node, [], [])
            |> of_tag
            |> as_raw_node,
          ),
          (
            " < Foo / > ",
            ("Foo" |> of_public |> as_raw_node, [], [])
            |> of_tag
            |> as_raw_node,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse fragment"
    >: (
      () =>
        [
          ("<></>", [] |> of_frag |> as_raw_node),
          (
            "<><Bar /></>",
            [
              ("Bar" |> of_public |> as_raw_node, [], [])
              |> jsx_node
              |> as_raw_node,
            ]
            |> of_frag
            |> as_raw_node,
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
              "Foo" |> of_public |> as_raw_node,
              [
                (
                  "fizz" |> of_public |> as_raw_node,
                  "buzz"
                  |> of_public
                  |> as_raw_node
                  |> of_id
                  |> as_raw_node
                  |> Option.some,
                )
                |> of_prop
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo fizz=\"buzz\" />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                (
                  "fizz" |> of_public |> as_raw_node,
                  "buzz" |> string_prim |> Option.some,
                )
                |> of_prop
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo fizz={ buzz; } />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                (
                  "fizz" |> of_public |> as_raw_node,
                  [
                    "buzz"
                    |> of_public
                    |> as_raw_node
                    |> of_id
                    |> as_raw_node
                    |> of_expr
                    |> as_raw_node,
                  ]
                  |> of_closure
                  |> as_raw_node
                  |> Option.some,
                )
                |> of_prop
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo fizz=1 + 2 />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                (
                  "fizz" |> of_public |> as_raw_node,
                  (1 |> int_prim, 2 |> int_prim)
                  |> of_add_op
                  |> as_raw_node
                  |> Option.some,
                )
                |> of_prop
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo fizz=(1 > 2) />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                (
                  "fizz" |> of_public |> as_raw_node,
                  (1 |> int_prim, 2 |> int_prim)
                  |> of_gt_op
                  |> as_raw_node
                  |> Option.some,
                )
                |> of_prop
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo fizz=(true) />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                (
                  "fizz" |> of_public |> as_raw_node,
                  bool_prim(true) |> Option.some,
                )
                |> of_prop
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo fizz=-3 />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                (
                  "fizz" |> of_public |> as_raw_node,
                  3 |> int_prim |> of_neg_op |> as_raw_node |> Option.some,
                )
                |> of_prop
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo fizz=<buzz /> />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                (
                  "fizz" |> of_public |> as_raw_node,
                  ("buzz" |> of_public |> as_raw_node, [], [])
                  |> jsx_tag
                  |> as_raw_node
                  |> Option.some,
                )
                |> of_prop
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo fizz />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                ("fizz" |> of_public |> as_raw_node, None)
                |> of_prop
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo .fizz />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                ("fizz" |> of_public |> as_raw_node, None)
                |> of_jsx_class
                |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo #fizz />",
            (
              "Foo" |> of_public |> as_raw_node,
              [
                "fizz" |> of_public |> as_raw_node |> of_jsx_id |> as_raw_node,
              ],
              [],
            )
            |> of_tag
            |> as_raw_node,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse single child"
    >: (
      () =>
        [
          (
            "<Foo><Bar /></Foo>",
            (
              "Foo" |> of_public |> as_raw_node,
              [],
              [
                ("Bar" |> of_public |> as_raw_node, [], [])
                |> jsx_node
                |> as_raw_node,
              ],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo>{1 + 2}</Foo>",
            (
              "Foo" |> of_public |> as_raw_node,
              [],
              [
                (1 |> int_prim, 2 |> int_prim)
                |> of_add_op
                |> as_raw_node
                |> of_inline_expr
                |> as_raw_node,
              ],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo>{<Bar />}</Foo>",
            (
              "Foo" |> of_public |> as_raw_node,
              [],
              [
                ("Bar" |> of_public |> as_raw_node, [], [])
                |> jsx_tag
                |> as_raw_node
                |> of_inline_expr
                |> as_raw_node,
              ],
            )
            |> of_tag
            |> as_raw_node,
          ),
          (
            "<Foo> bar \"or\" 123 </Foo>",
            (
              "Foo" |> of_public |> as_raw_node,
              [],
              ["bar \"or\" 123" |> as_raw_node |> of_text |> as_raw_node],
            )
            |> of_tag
            |> as_raw_node,
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
                "Foo" |> of_public |> as_raw_node,
                [
                  (
                    "bar" |> of_public |> as_raw_node,
                    4 |> int_prim |> Option.some,
                  )
                  |> of_prop
                  |> as_raw_node,
                ],
                [
                  ("Bar" |> of_public |> as_raw_node, [], [])
                  |> jsx_node
                  |> as_raw_node,
                ],
              )
              |> of_tag
              |> as_raw_node,
            ),
            (
              "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
              (
                "Foo" |> of_public |> as_raw_node,
                [],
                [
                  "bar" |> as_raw_node |> of_text |> as_raw_node,
                  (1 |> int_prim, 2 |> int_prim)
                  |> of_add_op
                  |> as_raw_node
                  |> of_inline_expr
                  |> as_raw_node,
                  ("Bar" |> of_public |> as_raw_node, [], [])
                  |> jsx_node
                  |> as_raw_node,
                  "fizz" |> string_prim |> of_inline_expr |> as_raw_node,
                  "buzz" |> as_raw_node |> of_text |> as_raw_node,
                ],
              )
              |> of_tag
              |> as_raw_node,
            ),
            (
              "<Foo bar=fizz .buzz />",
              (
                "Foo" |> of_public |> as_raw_node,
                [
                  (
                    "bar" |> of_public |> as_raw_node,
                    "fizz"
                    |> of_public
                    |> as_raw_node
                    |> of_id
                    |> as_raw_node
                    |> Option.some,
                  )
                  |> of_prop
                  |> as_raw_node,
                  ("buzz" |> of_public |> as_raw_node, None)
                  |> of_jsx_class
                  |> as_raw_node,
                ],
                [],
              )
              |> of_tag
              |> as_raw_node,
            ),
          ]
          |> Assert.parse_many
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
