open Kore;

module Formatter = Grammar.Formatter;
module U = Util.ResultUtil;

let _assert_jsx = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(pp_jsx));
let _assert_jsx_attr = (expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(pp_jsx_attr));

let suite =
  "Grammar.Formatter | JSX"
  >::: [
    "pp_jsx() - empty tag"
    >: (
      () =>
        _assert_jsx(
          "<Foo />",
          ("Foo" |> A.of_public |> U.as_raw_node, [], []) |> A.of_tag,
        )
    ),
    "pp_jsx() - empty component"
    >: (
      () =>
        _assert_jsx(
          "<Foo />",
          ("Foo" |> A.of_public |> U.as_view([], T.Valid(`Element)), [], [])
          |> A.of_component,
        )
    ),
    "pp_jsx() - empty fragment"
    >: (() => _assert_jsx("<></>", [] |> A.of_frag)),
    "pp_jsx() - tag with attributes"
    >: (
      () =>
        _assert_jsx(
          "<Foo #bar .fizz buzz />",
          (
            "Foo" |> A.of_public |> U.as_raw_node,
            [
              "bar"
              |> A.of_public
              |> U.as_raw_node
              |> A.of_jsx_id
              |> U.as_raw_node,
              ("fizz" |> A.of_public |> U.as_raw_node, None)
              |> A.of_jsx_class
              |> U.as_raw_node,
              ("buzz" |> A.of_public |> U.as_raw_node, None)
              |> A.of_prop
              |> U.as_raw_node,
            ],
            [],
          )
          |> A.of_tag,
        )
    ),
    "pp_jsx() - tag with nested text"
    >: (
      () =>
        _assert_jsx(
          "<Foo>
  bar
</Foo>",
          (
            "Foo" |> A.of_public |> U.as_raw_node,
            [],
            ["bar" |> A.of_text |> U.as_raw_node],
          )
          |> A.of_tag,
        )
    ),
    "pp_jsx() - tag with nested expression"
    >: (
      () =>
        _assert_jsx(
          "<Foo>
  {1 + 5}
</Foo>",
          (
            "Foo" |> A.of_public |> U.as_raw_node,
            [],
            [
              (1 |> U.int_prim, 5 |> U.int_prim)
              |> A.of_add_op
              |> U.as_int
              |> A.of_inline_expr
              |> U.as_raw_node,
            ],
          )
          |> A.of_tag,
        )
    ),
    "pp_jsx() - tag with nested tag"
    >: (
      () =>
        _assert_jsx(
          "<Foo>
  <Bar>
    fizzbuzz
  </Bar>
</Foo>",
          (
            "Foo" |> A.of_public |> U.as_raw_node,
            [],
            [
              (
                "Bar" |> A.of_public |> U.as_raw_node,
                [],
                ["fizzbuzz" |> A.of_text |> U.as_raw_node],
              )
              |> A.of_tag
              |> A.of_node
              |> U.as_raw_node,
            ],
          )
          |> A.of_tag,
        )
    ),
    "pp_jsx() - tag with multiple children"
    >: (
      () =>
        _assert_jsx(
          "<Foo>
  <Bar />
  {nil}
  Hello, World!
</Foo>",
          (
            "Foo" |> A.of_public |> U.as_raw_node,
            [],
            [
              ("Bar" |> A.of_public |> U.as_raw_node, [], [])
              |> A.of_tag
              |> A.of_node
              |> U.as_raw_node,
              U.nil_prim |> A.of_inline_expr |> U.as_raw_node,
              "Hello, World!" |> A.of_text |> U.as_raw_node,
            ],
          )
          |> A.of_tag,
        )
    ),
    "pp_jsx_attr() - property with primitive value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=123",
          ("fizz" |> A.of_public |> U.as_raw_node, Some(123 |> U.int_prim))
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with identifier value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=buzz",
          (
            "fizz" |> A.of_public |> U.as_raw_node,
            Some("buzz" |> A.of_public |> A.of_id |> U.as_int),
          )
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with binary operation value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=(1 + 2)",
          (
            "fizz" |> A.of_public |> U.as_raw_node,
            Some(
              (1 |> U.int_prim, 2 |> U.int_prim) |> A.of_add_op |> U.as_int,
            ),
          )
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with grouped binary operation value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=(1 + 2)",
          (
            "fizz" |> A.of_public |> U.as_raw_node,
            Some(
              (1 |> U.int_prim, 2 |> U.int_prim)
              |> A.of_add_op
              |> U.as_int
              |> A.of_group
              |> U.as_int,
            ),
          )
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with negative value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=(-1)",
          (
            "fizz" |> A.of_public |> U.as_raw_node,
            Some(1 |> U.int_prim |> A.of_neg_op |> U.as_int),
          )
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with boolean value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=true",
          (
            "fizz" |> A.of_public |> U.as_raw_node,
            Some(
              true
              |> U.bool_prim
              |> A.of_group
              |> U.as_bool
              |> A.of_group
              |> U.as_bool,
            ),
          )
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with closure value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz={
  true;
  false;
}",
          (
            "fizz" |> A.of_public |> U.as_raw_node,
            Some(
              [
                true |> U.bool_prim |> A.of_expr |> U.as_bool,
                false |> U.bool_prim |> A.of_expr |> U.as_bool,
              ]
              |> A.of_closure
              |> U.as_bool,
            ),
          )
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with inline JSX value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=<Buzz />",
          (
            "fizz" |> A.of_public |> U.as_raw_node,
            Some(
              ("Buzz" |> A.of_public |> U.as_raw_node, [], [])
              |> A.of_tag
              |> A.of_jsx
              |> U.as_element,
            ),
          )
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with multiline JSX value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=(<Buzz>
  <Foo />
</Buzz>)",
          (
            "fizz" |> A.of_public |> U.as_raw_node,
            Some(
              (
                "Buzz" |> A.of_public |> U.as_raw_node,
                [],
                [
                  ("Foo" |> A.of_public |> U.as_raw_node, [], [])
                  |> A.of_tag
                  |> A.of_node
                  |> U.as_raw_node,
                ],
              )
              |> A.of_tag
              |> A.of_jsx
              |> U.as_element,
            ),
          )
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with punned value"
    >: (
      () =>
        _assert_jsx_attr(
          "buzz",
          ("buzz" |> A.of_public |> U.as_raw_node, None) |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - dynamic class name"
    >: (
      () =>
        _assert_jsx_attr(
          ".fizz=true",
          ("fizz" |> A.of_public |> U.as_raw_node, Some(true |> U.bool_prim))
          |> A.of_jsx_class,
        )
    ),
    "pp_jsx_attr() - static class name"
    >: (
      () =>
        _assert_jsx_attr(
          ".fizz",
          ("fizz" |> A.of_public |> U.as_raw_node, None) |> A.of_jsx_class,
        )
    ),
    "pp_jsx_attr() - identifier name"
    >: (
      () =>
        _assert_jsx_attr(
          "#bar",
          "bar" |> A.of_public |> U.as_raw_node |> A.of_jsx_id,
        )
    ),
  ];
