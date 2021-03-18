open Kore;
open Util;
open AST;

module Expression = Grammar.Expression;
module JSX = Grammar.JSX;

module Assert =
  Assert.Make({
    type t = jsx_t;

    let parser = JSX.parser(Expression.parser) |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(pp => fmt_jsx % Format.pp_print_string(pp), (==)),
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
          ("<Foo></Foo>", of_tag(("Foo" |> of_public |> as_lexeme, [], []))),
          (
            " < Foo > < / Foo > ",
            of_tag(("Foo" |> of_public |> as_lexeme, [], [])),
          ),
        ]
        |> Assert.parse_many
    ),
    "parse self-closing tag"
    >: (
      () =>
        [
          ("<Foo/>", of_tag(("Foo" |> of_public |> as_lexeme, [], []))),
          (" < Foo / > ", of_tag(("Foo" |> of_public |> as_lexeme, [], []))),
        ]
        |> Assert.parse_many
    ),
    "parse fragment"
    >: (
      () =>
        [
          ("<></>", of_frag([])),
          (
            "<><Bar /></>",
            of_frag([("Bar" |> of_public |> as_lexeme, [], []) |> jsx_node]),
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
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  "buzz" |> of_public |> as_lexeme |> of_id |> some,
                )
                |> of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz=\"buzz\" />",
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  string_prim("buzz") |> some,
                )
                |> of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz={ buzz; } />",
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  ["buzz" |> of_public |> as_lexeme |> of_id |> of_expr]
                  |> to_block(~type_=Type.K_Unknown)
                  |> of_closure
                  |> some,
                )
                |> of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz=1 + 2 />",
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  (int_prim(1), int_prim(2)) |> of_add_op |> some,
                )
                |> of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz=(true) />",
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  bool_prim(true)
                  |> to_block(~type_=Type.K_Boolean)
                  |> of_group
                  |> some,
                )
                |> of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz=-3 />",
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  int_prim(3) |> of_neg_op |> some,
                )
                |> of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz=<buzz /> />",
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "fizz" |> of_public |> as_lexeme,
                  ("buzz" |> of_public |> as_lexeme, [], []) |> jsx |> some,
                )
                |> of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz />",
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              [("fizz" |> of_public |> as_lexeme, None) |> of_prop],
              [],
            )),
          ),
          (
            "<Foo .fizz />",
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              [("fizz" |> of_public |> as_lexeme, None) |> of_jsx_class],
              [],
            )),
          ),
          (
            "<Foo #fizz />",
            of_tag((
              "Foo" |> of_public |> as_lexeme,
              ["fizz" |> of_public |> as_lexeme |> of_jsx_id],
              [],
            )),
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
              "Foo" |> of_public |> as_lexeme,
              [],
              [("Bar" |> of_public |> as_lexeme, [], []) |> jsx_node],
            )
            |> of_tag,
          ),
          (
            "<Foo>{1 + 2}</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [(int_prim(1), int_prim(2)) |> of_add_op |> of_inline_expr],
            )
            |> of_tag,
          ),
          (
            "<Foo>{<Bar />}</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                ("Bar" |> of_public |> as_lexeme, [], [])
                |> jsx
                |> of_inline_expr,
              ],
            )
            |> of_tag,
          ),
          (
            "<Foo> bar \"or\" 123 </Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [of_text("bar \"or\" 123")],
            )
            |> of_tag,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse complex"
    >: (
      () =>
        [
          (
            "<Foo><Bar /></Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [("Bar" |> of_public |> as_lexeme, [], []) |> jsx_node],
            )
            |> of_tag,
          ),
          (
            "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                of_text("bar"),
                (int_prim(1), int_prim(2)) |> of_add_op |> of_inline_expr,
                ("Bar" |> of_public |> as_lexeme, [], []) |> jsx_node,
                string_prim("fizz") |> of_inline_expr,
                of_text("buzz"),
              ],
            )
            |> of_tag,
          ),
          (
            "<Foo bar=fizz .buzz />",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                (
                  "bar" |> of_public |> as_lexeme,
                  Some("fizz" |> of_public |> as_lexeme |> of_id),
                )
                |> of_prop,
                ("buzz" |> of_public |> as_lexeme, None) |> of_jsx_class,
              ],
              [],
            )
            |> of_tag,
          ),
        ]
        |> Assert.parse_many
    ),
  ];
