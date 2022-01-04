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
          testable(ppf => Dump.jsx_to_entity % Dump.Entity.pp(ppf), (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.JSX"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse tag"
    >: (
      () =>
        Assert.parse_all(
          ("Foo" |> of_public |> as_raw_node, [], []) |> of_tag |> as_raw_node,
          ["<Foo></Foo>", " < Foo > < / Foo > "],
        )
    ),
    "parse self-closing tag"
    >: (
      () =>
        Assert.parse_all(
          ("Foo" |> of_public |> as_raw_node, [], []) |> of_tag |> as_raw_node,
          ["<Foo/>", " < Foo / > "],
        )
    ),
    "parse empty fragment"
    >: (() => Assert.parse([] |> of_frag |> as_raw_node, "<></>")),
    "parse fragment with children"
    >: (
      () =>
        Assert.parse(
          [
            ("Bar" |> of_public |> as_raw_node, [], [])
            |> jsx_node
            |> as_raw_node,
          ]
          |> of_frag
          |> as_raw_node,
          "<><Bar /></>",
        )
    ),
    "parse property with identifier value"
    >: (
      () =>
        Assert.parse(
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
          "<Foo fizz=buzz />",
        )
    ),
    "parse property with string value"
    >: (
      () =>
        Assert.parse(
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
          "<Foo fizz=\"buzz\" />",
        )
    ),
    "parse property with closure value"
    >: (
      () =>
        Assert.parse(
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
          "<Foo fizz={ buzz; } />",
        )
    ),
    "parse property with expression value"
    >: (
      () =>
        Assert.parse(
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
          "<Foo fizz=1 + 2 />",
        )
    ),
    "parse property with grouped expression value"
    >: (
      () =>
        Assert.parse(
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
          "<Foo fizz=(1 > 2) />",
        )
    ),
    "parse property with grouped boolean value"
    >: (
      () =>
        Assert.parse(
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
          "<Foo fizz=(true) />",
        )
    ),
    "parse property with negative integer value"
    >: (
      () =>
        Assert.parse(
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
          "<Foo fizz=-3 />",
        )
    ),
    "parse property with JSX value"
    >: (
      () =>
        Assert.parse(
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
          "<Foo fizz=<buzz /> />",
        )
    ),
    "parse property with punned value"
    >: (
      () =>
        Assert.parse(
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
          "<Foo fizz />",
        )
    ),
    "parse property with static class name"
    >: (
      () =>
        Assert.parse(
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
          "<Foo .fizz />",
        )
    ),
    "parse property with identifier"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> of_public |> as_raw_node,
            ["fizz" |> of_public |> as_raw_node |> of_jsx_id |> as_raw_node],
            [],
          )
          |> of_tag
          |> as_raw_node,
          "<Foo #fizz />",
        )
    ),
    "parse single tag child"
    >: (
      () =>
        Assert.parse(
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
          "<Foo><Bar /></Foo>",
        )
    ),
    "parse single inline expression child"
    >: (
      () =>
        Assert.parse(
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
          "<Foo>{1 + 2}</Foo>",
        )
    ),
    "parse single JSX inline expression child"
    >: (
      () =>
        Assert.parse(
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
          "<Foo>{<Bar />}</Foo>",
        )
    ),
    "parse single text child"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> of_public |> as_raw_node,
            [],
            ["bar \"or\" 123" |> as_raw_node |> of_text |> as_raw_node],
          )
          |> of_tag
          |> as_raw_node,
          "<Foo> bar \"or\" 123 </Foo>",
        )
    ),
    "parse complex - nested with attributes"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> of_public |> as_raw_node,
            [
              ("bar" |> of_public |> as_raw_node, 4 |> int_prim |> Option.some)
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
          "<Foo bar=4><Bar /></Foo>",
        )
    ),
    "parse complex - multiple inline children of different types"
    >: (
      () =>
        Assert.parse(
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
          "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
        )
    ),
    "parse complex - multiple attributes different types"
    >: (
      () =>
        Assert.parse(
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
          "<Foo bar=fizz .buzz />",
        )
    ),
  ];
