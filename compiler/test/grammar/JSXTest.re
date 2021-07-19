open Kore;
open AST.Raw.Util;
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

let _to_prop_lexeme = to_prop % as_lexeme;
let _to_tag_lexeme = to_tag % as_lexeme;
let _to_text_lexeme = as_lexeme % to_text % as_lexeme;
let _to_frag_lexeme = to_frag % as_lexeme;
let _to_jsx_node_lexeme = jsx_node % as_lexeme;

let suite =
  "Grammar.JSX"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse tag"
    >: (
      () =>
        [
          ("<Foo></Foo>", (raw_public("Foo"), [], []) |> _to_tag_lexeme),
          (
            " < Foo > < / Foo > ",
            (raw_public("Foo"), [], []) |> _to_tag_lexeme,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse self-closing tag"
    >: (
      () =>
        [
          ("<Foo/>", (raw_public("Foo"), [], []) |> _to_tag_lexeme),
          (" < Foo / > ", (raw_public("Foo"), [], []) |> _to_tag_lexeme),
        ]
        |> Assert.parse_many
    ),
    "parse fragment"
    >: (
      () =>
        [
          ("<></>", [] |> _to_frag_lexeme),
          (
            "<><Bar /></>",
            [(raw_public("Bar"), [], []) |> _to_jsx_node_lexeme]
            |> _to_frag_lexeme,
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
              raw_public("Foo"),
              [
                (
                  raw_public("fizz"),
                  "buzz" |> to_public_id(as_bool) |> Option.some,
                )
                |> _to_prop_lexeme,
              ],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo fizz=\"buzz\" />",
            (
              raw_public("Foo"),
              [
                (raw_public("fizz"), "buzz" |> string_prim |> Option.some)
                |> _to_prop_lexeme,
              ],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo fizz={ buzz; } />",
            (
              raw_public("Foo"),
              [
                (
                  raw_public("fizz"),
                  ["buzz" |> to_public_id(as_bool) |> to_expr]
                  |> to_closure
                  |> as_bool
                  |> Option.some,
                )
                |> _to_prop_lexeme,
              ],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo fizz=1 + 2 />",
            (
              raw_public("Foo"),
              [
                (
                  raw_public("fizz"),
                  (1 |> int_prim, 2 |> int_prim)
                  |> to_add_op
                  |> as_int
                  |> Option.some,
                )
                |> _to_prop_lexeme,
              ],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo fizz=(1 > 2) />",
            (
              raw_public("Foo"),
              [
                (
                  raw_public("fizz"),
                  (int_prim(1), int_prim(2))
                  |> to_gt_op
                  |> as_bool
                  |> Option.some,
                )
                |> _to_prop_lexeme,
              ],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo fizz=(true) />",
            (
              raw_public("Foo"),
              [
                (raw_public("fizz"), bool_prim(true) |> Option.some)
                |> _to_prop_lexeme,
              ],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo fizz=-3 />",
            (
              raw_public("Foo"),
              [
                (
                  raw_public("fizz"),
                  3 |> int_prim |> to_neg_int |> Option.some,
                )
                |> _to_prop_lexeme,
              ],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo fizz=<buzz /> />",
            (
              raw_public("Foo"),
              [
                (
                  raw_public("fizz"),
                  (raw_public("buzz"), [], [])
                  |> jsx_tag
                  |> as_element
                  |> Option.some,
                )
                |> _to_prop_lexeme,
              ],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo fizz />",
            (
              raw_public("Foo"),
              [(raw_public("fizz"), None) |> _to_prop_lexeme],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo .fizz />",
            (
              raw_public("Foo"),
              [(raw_public("fizz"), None) |> to_jsx_class |> as_lexeme],
              [],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo #fizz />",
            (
              raw_public("Foo"),
              ["fizz" |> raw_public |> to_jsx_id |> as_lexeme],
              [],
            )
            |> _to_tag_lexeme,
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
              raw_public("Foo"),
              [],
              [(raw_public("Bar"), [], []) |> _to_jsx_node_lexeme],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo>{1 + 2}</Foo>",
            (
              "Foo" |> to_public |> as_lexeme,
              [],
              [
                (1 |> int_prim, 2 |> int_prim)
                |> to_add_op
                |> as_int
                |> to_inline_expr
                |> as_lexeme,
              ],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo>{<Bar />}</Foo>",
            (
              "Foo" |> to_public |> as_lexeme,
              [],
              [
                ("Bar" |> to_public |> as_lexeme, [], [])
                |> jsx_tag
                |> as_element
                |> to_inline_expr
                |> as_lexeme,
              ],
            )
            |> _to_tag_lexeme,
          ),
          (
            "<Foo> bar \"or\" 123 </Foo>",
            (raw_public("Foo"), [], [_to_text_lexeme("bar \"or\" 123")])
            |> _to_tag_lexeme,
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
                raw_public("Foo"),
                [
                  (raw_public("bar"), 4 |> int_prim |> Option.some)
                  |> _to_prop_lexeme,
                ],
                [(raw_public("Bar"), [], []) |> _to_jsx_node_lexeme],
              )
              |> _to_tag_lexeme,
            ),
            (
              "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
              (
                raw_public("Foo"),
                [],
                [
                  _to_text_lexeme("bar"),
                  (int_prim(1), int_prim(2))
                  |> to_add_op
                  |> as_int
                  |> to_inline_expr
                  |> as_lexeme,
                  (raw_public("Bar"), [], []) |> _to_jsx_node_lexeme,
                  "fizz" |> string_prim |> to_inline_expr |> as_lexeme,
                  _to_text_lexeme("buzz"),
                ],
              )
              |> _to_tag_lexeme,
            ),
            (
              "<Foo bar=fizz .buzz />",
              (
                raw_public("Foo"),
                [
                  (
                    raw_public("bar"),
                    "fizz" |> to_public_id(as_bool) |> Option.some,
                  )
                  |> _to_prop_lexeme,
                  (raw_public("buzz"), None) |> to_jsx_class |> as_lexeme,
                ],
                [],
              )
              |> _to_tag_lexeme,
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
