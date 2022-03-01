open Kore;

module Expression = Grammar.Expression;
module JSX = Grammar.JSX;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = NR.t(AR.jsx_t);

    let parser = ((_, ctx)) =>
      JSX.parser(ctx, (Expression.expr_4, Expression.parser))
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, jsx) =>
              AR.Dump.(
                jsx
                |> untyped_node_to_entity(
                     ~children=[jsx |> NR.get_value |> jsx_to_entity],
                     "JSX",
                   )
                |> Entity.pp(ppf)
              ),
            (==),
          ),
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
          ("Foo" |> AR.of_public |> U.as_raw_node, [], [])
          |> AR.of_tag
          |> U.as_raw_node,
          ["<Foo></Foo>", " < Foo > < / Foo > "],
        )
    ),
    "parse self-closing tag"
    >: (
      () =>
        Assert.parse_all(
          ("Foo" |> AR.of_public |> U.as_raw_node, [], [])
          |> AR.of_tag
          |> U.as_raw_node,
          ["<Foo/>", " < Foo / > "],
        )
    ),
    "parse empty fragment"
    >: (() => Assert.parse([] |> AR.of_frag |> U.as_raw_node, "<></>")),
    "parse fragment with children"
    >: (
      () =>
        Assert.parse(
          [
            ("Bar" |> AR.of_public |> U.as_raw_node, [], [])
            |> U.jsx_node
            |> U.as_raw_node,
          ]
          |> AR.of_frag
          |> U.as_raw_node,
          "<><Bar /></>",
        )
    ),
    "parse property with identifier value"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "fizz" |> AR.of_public |> U.as_raw_node,
                "buzz"
                |> AR.of_public
                |> AR.of_id
                |> U.as_unknown
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo fizz=buzz />",
        )
    ),
    "parse property with string value"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "fizz" |> AR.of_public |> U.as_raw_node,
                "buzz" |> U.string_prim |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo fizz=\"buzz\" />",
        )
    ),
    "parse property with closure value"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "fizz" |> AR.of_public |> U.as_raw_node,
                [
                  "buzz"
                  |> AR.of_public
                  |> AR.of_id
                  |> U.as_unknown
                  |> AR.of_expr
                  |> U.as_unknown,
                ]
                |> AR.of_closure
                |> U.as_unknown
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo fizz={ buzz; } />",
        )
    ),
    "parse property with expression value"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "fizz" |> AR.of_public |> U.as_raw_node,
                (U.int_prim(1), U.int_prim(2))
                |> AR.of_add_op
                |> U.as_int
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo fizz=1 + 2 />",
        )
    ),
    "parse property with grouped expression value"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "fizz" |> AR.of_public |> U.as_raw_node,
                (U.int_prim(1), U.int_prim(2))
                |> AR.of_gt_op
                |> U.as_bool
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo fizz=(1 > 2) />",
        )
    ),
    "parse property with grouped boolean value"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "fizz" |> AR.of_public |> U.as_raw_node,
                true |> U.bool_prim |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo fizz=(true) />",
        )
    ),
    "parse property with negative integer value"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "fizz" |> AR.of_public |> U.as_raw_node,
                3 |> U.int_prim |> AR.of_neg_op |> U.as_int |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo fizz=-3 />",
        )
    ),
    "parse property with JSX value"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "fizz" |> AR.of_public |> U.as_raw_node,
                ("buzz" |> AR.of_public |> U.as_raw_node, [], [])
                |> U.jsx_tag
                |> U.as_element
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo fizz=<buzz /> />",
        )
    ),
    "parse property with punned value"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              ("fizz" |> AR.of_public |> U.as_raw_node, None)
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo fizz />",
        )
    ),
    "parse property with static class name"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              ("fizz" |> AR.of_public |> U.as_raw_node, None)
              |> AR.of_jsx_class
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo .fizz />",
        )
    ),
    "parse property with identifier"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              "fizz"
              |> AR.of_public
              |> U.as_raw_node
              |> AR.of_jsx_id
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo #fizz />",
        )
    ),
    "parse single tag child"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [],
            [
              ("Bar" |> AR.of_public |> U.as_raw_node, [], [])
              |> U.jsx_node
              |> U.as_raw_node,
            ],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo><Bar /></Foo>",
        )
    ),
    "parse single inline expression child"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [],
            [
              (U.int_prim(1), U.int_prim(2))
              |> AR.of_add_op
              |> U.as_int
              |> AR.of_inline_expr
              |> U.as_raw_node,
            ],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo>{1 + 2}</Foo>",
        )
    ),
    "parse single JSX inline expression child"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [],
            [
              ("Bar" |> AR.of_public |> U.as_raw_node, [], [])
              |> U.jsx_tag
              |> U.as_element
              |> AR.of_inline_expr
              |> U.as_raw_node,
            ],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo>{<Bar />}</Foo>",
        )
    ),
    "parse single text child"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [],
            ["bar \"or\" 123" |> AR.of_text |> U.as_raw_node],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo> bar \"or\" 123 </Foo>",
        )
    ),
    "parse complex - nested with attributes"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "bar" |> AR.of_public |> U.as_raw_node,
                4 |> U.int_prim |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
            ],
            [
              ("Bar" |> AR.of_public |> U.as_raw_node, [], [])
              |> U.jsx_node
              |> U.as_raw_node,
            ],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo bar=4><Bar /></Foo>",
        )
    ),
    "parse complex - multiple inline children of different types"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [],
            [
              "bar" |> AR.of_text |> U.as_raw_node,
              (U.int_prim(1), U.int_prim(2))
              |> AR.of_add_op
              |> U.as_int
              |> AR.of_inline_expr
              |> U.as_raw_node,
              ("Bar" |> AR.of_public |> U.as_raw_node, [], [])
              |> U.jsx_node
              |> U.as_raw_node,
              "fizz" |> U.string_prim |> AR.of_inline_expr |> U.as_raw_node,
              "buzz" |> AR.of_text |> U.as_raw_node,
            ],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
        )
    ),
    "parse complex - multiple attributes different types"
    >: (
      () =>
        Assert.parse(
          (
            "Foo" |> AR.of_public |> U.as_raw_node,
            [
              (
                "bar" |> AR.of_public |> U.as_raw_node,
                "fizz"
                |> AR.of_public
                |> AR.of_id
                |> U.as_unknown
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_raw_node,
              ("buzz" |> AR.of_public |> U.as_raw_node, None)
              |> AR.of_jsx_class
              |> U.as_raw_node,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_raw_node,
          "<Foo bar=fizz .buzz />",
        )
    ),
  ];
