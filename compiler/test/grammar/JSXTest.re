open Kore;

module AR = AST.Raw;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = AR.expression_t;

    let parser = ctx =>
      KSX.Plugin.parse(
        ctx,
        (KExpression.Plugin.parse_jsx_term, KExpression.Plugin.parse),
      )
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf => KExpression.Plugin.to_xml(_ => "") % Fmt.xml_string(ppf),
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
          (U.as_untyped("Foo"), [], [])
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          ["<Foo></Foo>", " < Foo > < / Foo > "],
        )
    ),
    "parse self-closing tag"
    >: (
      () =>
        Assert.parse_all(
          (U.as_untyped("Foo"), [], [])
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          ["<Foo/>", " < Foo / > "],
        )
    ),
    "parse empty fragment"
    >: (
      () => Assert.parse([] |> AR.of_frag |> AR.of_jsx |> U.as_node, "<></>")
    ),
    "parse fragment with children"
    >: (
      () =>
        Assert.parse(
          [(U.as_untyped("Bar"), [], []) |> U.jsx_node |> U.as_untyped]
          |> AR.of_frag
          |> AR.of_jsx
          |> U.as_node,
          "<><Bar /></>",
        )
    ),
    "parse property with identifier value"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (
                U.as_untyped("fizz"),
                "buzz" |> AR.of_id |> U.as_node |> Option.some,
              )
              |> AR.of_prop
              |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo fizz=buzz />",
        )
    ),
    "parse property with string value"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (U.as_untyped("fizz"), "buzz" |> U.string_prim |> Option.some)
              |> AR.of_prop
              |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo fizz=\"buzz\" />",
        )
    ),
    "parse property with closure value"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (
                U.as_untyped("fizz"),
                ["buzz" |> AR.of_id |> U.as_node |> AR.of_expr |> U.as_node]
                |> AR.of_closure
                |> U.as_node
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo fizz={ buzz; } />",
        )
    ),
    "parse property with expression value"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (
                U.as_untyped("fizz"),
                ("buzz" |> AR.of_id |> U.as_node, [])
                |> AR.of_func_call
                |> U.as_node
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo fizz=buzz() />",
        )
    ),
    "parse property with grouped expression value"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (
                U.as_untyped("fizz"),
                (U.int_prim(1), U.int_prim(2))
                |> AR.of_gt_op
                |> U.as_node
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo fizz=(1 > 2) />",
        )
    ),
    "parse property with grouped boolean value"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (U.as_untyped("fizz"), true |> U.bool_prim |> Option.some)
              |> AR.of_prop
              |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo fizz=(true) />",
        )
    ),
    "parse property with negative integer value"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (
                U.as_untyped("fizz"),
                3 |> U.int_prim |> AR.of_neg_op |> U.as_node |> Option.some,
              )
              |> AR.of_prop
              |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo fizz=-3 />",
        )
    ),
    "parse property with JSX value"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (
                U.as_untyped("fizz"),
                (U.as_untyped("buzz"), [], [])
                |> U.jsx_tag
                |> U.as_node
                |> Option.some,
              )
              |> AR.of_prop
              |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo fizz=<buzz /> />",
        )
    ),
    "parse property with punned value"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [(U.as_untyped("fizz"), None) |> AR.of_prop |> U.as_untyped],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo fizz />",
        )
    ),
    "parse property with static class name"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (U.as_untyped("fizz"), None) |> AR.of_jsx_class |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo .fizz />",
        )
    ),
    "parse property with identifier"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            ["fizz" |> U.as_untyped |> AR.of_jsx_id |> U.as_untyped],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo #fizz />",
        )
    ),
    "parse single tag child"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [],
            [(U.as_untyped("Bar"), [], []) |> U.jsx_node |> U.as_untyped],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo><Bar /></Foo>",
        )
    ),
    "parse single inline expression child"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (U.int_prim(1), U.int_prim(2))
              |> AR.of_add_op
              |> U.as_node
              |> AR.of_inline_expr
              |> U.as_untyped,
            ],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo>{1 + 2}</Foo>",
        )
    ),
    "parse single JSX inline expression child"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (U.as_untyped("Bar"), [], [])
              |> U.jsx_tag
              |> U.as_node
              |> AR.of_inline_expr
              |> U.as_untyped,
            ],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo>{<Bar />}</Foo>",
        )
    ),
    "parse single text child"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [],
            ["bar \"or\" 123" |> AR.of_text |> U.as_untyped],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo> bar \"or\" 123 </Foo>",
        )
    ),
    "parse complex - nested with attributes"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (U.as_untyped("bar"), 4 |> U.int_prim |> Option.some)
              |> AR.of_prop
              |> U.as_untyped,
            ],
            [(U.as_untyped("Bar"), [], []) |> U.jsx_node |> U.as_untyped],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo bar=4><Bar /></Foo>",
        )
    ),
    "parse complex - multiple inline children of different types"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              "bar" |> AR.of_text |> U.as_untyped,
              (U.int_prim(1), U.int_prim(2))
              |> AR.of_add_op
              |> U.as_node
              |> AR.of_inline_expr
              |> U.as_untyped,
              (U.as_untyped("Bar"), [], []) |> U.jsx_node |> U.as_untyped,
              "fizz" |> U.string_prim |> AR.of_inline_expr |> U.as_untyped,
              "buzz" |> AR.of_text |> U.as_untyped,
            ],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
        )
    ),
    "parse complex - multiple attributes different types"
    >: (
      () =>
        Assert.parse(
          (
            U.as_untyped("Foo"),
            [
              (
                U.as_untyped("bar"),
                "fizz" |> AR.of_id |> U.as_node |> Option.some,
              )
              |> AR.of_prop
              |> U.as_untyped,
              (U.as_untyped("buzz"), None) |> AR.of_jsx_class |> U.as_untyped,
            ],
            [],
          )
          |> AR.of_tag
          |> AR.of_jsx
          |> U.as_node,
          "<Foo bar=fizz .buzz />",
        )
    ),
  ];
