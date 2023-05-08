open Kore;

module U = Util.RawUtil;

let suite =
  "Grammar.JSX"
  >::: [
    "no parse" >: (() => Assert.Expression.no_parse("<gibberish")),
    "parse tag"
    >: (
      () =>
        Assert.Expression.parse_all(
          (U.as_untyped("Foo"), [], [], [])
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          ["<Foo></Foo>", " < Foo > < / Foo > "],
        )
    ),
    "parse self-closing tag"
    >: (
      () =>
        Assert.Expression.parse_all(
          (U.as_untyped("Foo"), [], [], [])
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          ["<Foo/>", " < Foo / > "],
        )
    ),
    "parse style expression binding"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            ["bar" |> Expression.of_identifier |> U.as_untyped],
            [],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo::bar />",
        )
    ),
    "parse style literal binding"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [
              [
                (U.as_untyped("color"), U.string_prim("red")) |> U.as_untyped,
              ]
              |> Expression.of_style
              |> U.as_untyped,
            ],
            [],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo::{ color: \"red\" } />",
        )
    ),
    "parse empty fragment"
    >: (
      () =>
        Assert.Expression.parse(
          [] |> KSX.of_fragment |> Expression.of_ksx |> U.as_node,
          "<></>",
        )
    ),
    "parse fragment with children"
    >: (
      () =>
        Assert.Expression.parse(
          [(U.as_untyped("Bar"), [], [], []) |> U.ksx_node |> U.as_untyped]
          |> KSX.of_fragment
          |> Expression.of_ksx
          |> U.as_node,
          "<><Bar /></>",
        )
    ),
    "parse property with identifier value"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (
                U.as_untyped("fizz"),
                "buzz" |> Expression.of_identifier |> U.as_node |> Option.some,
              )
              |> U.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo fizz=buzz />",
        )
    ),
    "parse property with string value"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (U.as_untyped("fizz"), "buzz" |> U.string_prim |> Option.some)
              |> U.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo fizz=\"buzz\" />",
        )
    ),
    "parse property with closure value"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (
                U.as_untyped("fizz"),
                [
                  "buzz"
                  |> Expression.of_identifier
                  |> U.as_node
                  |> Statement.of_effect
                  |> U.as_node,
                ]
                |> Expression.of_closure
                |> U.as_node
                |> Option.some,
              )
              |> U.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo fizz={ buzz; } />",
        )
    ),
    "parse property with expression value"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (
                U.as_untyped("fizz"),
                ("buzz" |> Expression.of_identifier |> U.as_node, [])
                |> Expression.of_function_call
                |> U.as_node
                |> Option.some,
              )
              |> U.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo fizz=buzz() />",
        )
    ),
    "parse property with grouped expression value"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (
                U.as_untyped("fizz"),
                (U.int_prim(1), U.int_prim(2))
                |> Expression.of_gt_op
                |> U.as_node
                |> Option.some,
              )
              |> U.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo fizz=(1 > 2) />",
        )
    ),
    "parse property with grouped boolean value"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (U.as_untyped("fizz"), true |> U.bool_prim |> Option.some)
              |> U.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo fizz=(true) />",
        )
    ),
    "parse property with negative integer value"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (
                U.as_untyped("fizz"),
                3
                |> U.int_prim
                |> Expression.of_negative_op
                |> U.as_node
                |> Option.some,
              )
              |> U.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo fizz=-3 />",
        )
    ),
    "parse property with JSX value"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (
                U.as_untyped("fizz"),
                (U.as_untyped("buzz"), [], [], [])
                |> U.ksx_tag
                |> U.as_node
                |> Option.some,
              )
              |> U.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo fizz=<buzz /> />",
        )
    ),
    "parse property with punned value"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [(U.as_untyped("fizz"), None) |> U.as_untyped],
            [],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo fizz />",
        )
    ),
    "parse single tag child"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [],
            [
              (U.as_untyped("Bar"), [], [], []) |> U.ksx_node |> U.as_untyped,
            ],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo><Bar /></Foo>",
        )
    ),
    "parse single inline expression child"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [],
            [
              (U.int_prim(1), U.int_prim(2))
              |> Expression.of_add_op
              |> U.as_node
              |> KSX.Child.of_inline
              |> U.as_untyped,
            ],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo>{1 + 2}</Foo>",
        )
    ),
    "parse single JSX inline expression child"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [],
            [
              (U.as_untyped("Bar"), [], [], [])
              |> U.ksx_tag
              |> U.as_node
              |> KSX.Child.of_inline
              |> U.as_untyped,
            ],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo>{<Bar />}</Foo>",
        )
    ),
    "parse single text child"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [],
            ["bar \"or\" 123" |> KSX.Child.of_text |> U.as_untyped],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo> bar \"or\" 123 </Foo>",
        )
    ),
    "parse complex - nested with attributes"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [
              (U.as_untyped("bar"), 4 |> U.int_prim |> Option.some)
              |> U.as_untyped,
            ],
            [
              (U.as_untyped("Bar"), [], [], []) |> U.ksx_node |> U.as_untyped,
            ],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo bar=4><Bar /></Foo>",
        )
    ),
    "parse complex - multiple inline children of different types"
    >: (
      () =>
        Assert.Expression.parse(
          (
            U.as_untyped("Foo"),
            [],
            [],
            [
              "bar" |> KSX.Child.of_text |> U.as_untyped,
              (U.int_prim(1), U.int_prim(2))
              |> Expression.of_add_op
              |> U.as_node
              |> KSX.Child.of_inline
              |> U.as_untyped,
              (U.as_untyped("Bar"), [], [], []) |> U.ksx_node |> U.as_untyped,
              "fizz" |> U.string_prim |> KSX.Child.of_inline |> U.as_untyped,
              "buzz" |> KSX.Child.of_text |> U.as_untyped,
            ],
          )
          |> KSX.of_element_tag
          |> Expression.of_ksx
          |> U.as_node,
          "<Foo>bar{1 + 2}<Bar />{\"fizz\"}buzz</Foo>",
        )
    ),
  ];
