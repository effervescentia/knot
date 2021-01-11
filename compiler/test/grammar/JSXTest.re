open Kore;
open Util;

module Expression = Grammar.Expression;
module JSX = Grammar.JSX;

module Assert =
  Assert.Make({
    type t = AST.jsx_t;

    let parser = JSX.parser(Expression.parser) |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(_ => fmt_jsx % Format.print_string, (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar - JSX"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse tag"
    >: (
      () =>
        [
          ("<Foo></Foo>", AST.of_tag(("Foo", [], []))),
          (" < Foo > < / Foo > ", AST.of_tag(("Foo", [], []))),
        ]
        |> Assert.parse_many
    ),
    "parse self-closing tag"
    >: (
      () =>
        [
          ("<Foo/>", AST.of_tag(("Foo", [], []))),
          (" < Foo / > ", AST.of_tag(("Foo", [], []))),
        ]
        |> Assert.parse_many
    ),
    "parse fragment"
    >: (
      () =>
        [
          ("<></>", AST.of_frag([])),
          ("<><Bar /></>", AST.of_frag([("Bar", [], []) |> jsx_node])),
        ]
        |> Assert.parse_many
    ),
    "parse attributes"
    >: (
      () =>
        [
          (
            "<Foo fizz=buzz />",
            AST.of_tag((
              "Foo",
              [("fizz", inv_id("buzz") |> some) |> AST.of_prop],
              [],
            )),
          ),
          (
            "<Foo fizz=\"buzz\" />",
            AST.of_tag((
              "Foo",
              [("fizz", string_prim("buzz") |> some) |> AST.of_prop],
              [],
            )),
          ),
          (
            "<Foo fizz={ buzz; } />",
            AST.of_tag((
              "Foo",
              [
                (
                  "fizz",
                  [inv_id("buzz") |> AST.of_expr] |> AST.of_closure |> some,
                )
                |> AST.of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz=1 + 2 />",
            AST.of_tag((
              "Foo",
              [
                (
                  "fizz",
                  (int_prim(1), int_prim(2)) |> AST.of_add_op |> some,
                )
                |> AST.of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz=(true) />",
            AST.of_tag((
              "Foo",
              [
                ("fizz", bool_prim(true) |> AST.of_group |> some)
                |> AST.of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz=-3 />",
            AST.of_tag((
              "Foo",
              [
                ("fizz", int_prim(3) |> AST.of_neg_op |> some) |> AST.of_prop,
              ],
              [],
            )),
          ),
          (
            "<Foo fizz=<buzz /> />",
            AST.of_tag((
              "Foo",
              [("fizz", ("buzz", [], []) |> jsx |> some) |> AST.of_prop],
              [],
            )),
          ),
          (
            "<Foo fizz />",
            AST.of_tag(("Foo", [("fizz", None) |> AST.of_prop], [])),
          ),
          (
            "<Foo .fizz />",
            AST.of_tag(("Foo", [("fizz", None) |> AST.of_jsx_class], [])),
          ),
          (
            "<Foo #fizz />",
            AST.of_tag(("Foo", [("fizz", None) |> AST.of_jsx_id], [])),
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
            ("Foo", [], [("Bar", [], []) |> jsx_node]) |> AST.of_tag,
          ),
          (
            "<Foo>{1 + 2}</Foo>",
            (
              "Foo",
              [],
              [
                (int_prim(1), int_prim(2))
                |> AST.of_add_op
                |> AST.of_inline_expr,
              ],
            )
            |> AST.of_tag,
          ),
          (
            "<Foo>{<Bar />}</Foo>",
            ("Foo", [], [("Bar", [], []) |> jsx |> AST.of_inline_expr])
            |> AST.of_tag,
          ),
          (
            "<Foo> bar \"or\" 123 </Foo>",
            ("Foo", [], [AST.of_text("bar \"or\" 123")]) |> AST.of_tag,
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
            ("Foo", [], [("Bar", [], []) |> jsx_node]) |> AST.of_tag,
          ),
          (
            "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
            (
              "Foo",
              [],
              [
                AST.of_text("bar"),
                (int_prim(1), int_prim(2))
                |> AST.of_add_op
                |> AST.of_inline_expr,
                ("Bar", [], []) |> jsx_node,
                string_prim("fizz") |> AST.of_inline_expr,
                AST.of_text("buzz"),
              ],
            )
            |> AST.of_tag,
          ),
        ]
        |> Assert.parse_many
    ),
  ];
