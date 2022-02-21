open Kore;

module Expression = Grammar.ExpressionV2;
module JSX = Grammar.JSXV2;
module U = Util.RawUtilV2;

module Assert =
  Assert.Make({
    type t = AR.jsx_t;

    let parser = ((_, ctx)) =>
      JSX.parser(ctx, (Expression.expr_4, Expression.parser)) |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(ppf => AR.Dump.(jsx_to_entity % Entity.pp(ppf)), (==)),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.JSXV2"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse tag"
    >: (
      () =>
        Assert.parse_all(
          ("Foo" |> AR.of_public |> U.as_raw_node, [], [])
          |> AR.of_tag
          |> U.as_element,
          ["<Foo></Foo>", " < Foo > < / Foo > "],
        )
    ),
    "parse self-closing tag"
    >: (
      () =>
        Assert.parse_all(
          ("Foo" |> AR.of_public |> U.as_raw_node, [], [])
          |> AR.of_tag
          |> U.as_element,
          ["<Foo/>", " < Foo / > "],
        )
    ),
    "parse empty fragment"
    >: (() => Assert.parse([] |> AR.of_frag |> U.as_element, "<></>")),
    "parse fragment with children"
    >: (
      () =>
        Assert.parse(
          [
            ("Bar" |> AR.of_public |> U.as_raw_node, [], [])
            |> U.jsx_node
            |> U.as_element,
          ]
          |> AR.of_frag
          |> U.as_element,
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
                |> U.as_raw_node
                |> AR.of_id
                |> U.as_unknown
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_unknown,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_string,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
                  |> U.as_raw_node
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
              |> U.as_unknown,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_int,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_bool,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_bool,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_int,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_element,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_unknown,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_bool,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_string,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_element,
            ],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_int,
            ],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_element,
            ],
          )
          |> AR.of_tag
          |> U.as_element,
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
            ["bar \"or\" 123" |> U.as_string |> AR.of_text |> U.as_string],
          )
          |> AR.of_tag
          |> U.as_element,
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
              |> U.as_int,
            ],
            [
              ("Bar" |> AR.of_public |> U.as_raw_node, [], [])
              |> U.jsx_node
              |> U.as_element,
            ],
          )
          |> AR.of_tag
          |> U.as_element,
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
              "bar" |> U.as_string |> AR.of_text |> U.as_string,
              (U.int_prim(1), U.int_prim(2))
              |> AR.of_add_op
              |> U.as_int
              |> AR.of_inline_expr
              |> U.as_int,
              ("Bar" |> AR.of_public |> U.as_raw_node, [], [])
              |> U.jsx_node
              |> U.as_element,
              "fizz" |> U.string_prim |> AR.of_inline_expr |> U.as_string,
              "buzz" |> U.as_string |> AR.of_text |> U.as_string,
            ],
          )
          |> AR.of_tag
          |> U.as_element,
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
                |> U.as_raw_node
                |> AR.of_id
                |> U.as_unknown
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_unknown,
              ("buzz" |> AR.of_public |> U.as_raw_node, None)
              |> AR.of_jsx_class
              |> U.as_bool,
            ],
            [],
          )
          |> AR.of_tag
          |> U.as_element,
          "<Foo bar=fizz .buzz />",
        )
    ),
  ];
