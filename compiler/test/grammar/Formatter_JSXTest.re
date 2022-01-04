open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Formatter = Grammar.Formatter;

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
          ("Foo" |> of_public |> as_raw_node, [], []) |> of_tag,
        )
    ),
    "pp_jsx() - empty fragment" >: (() => _assert_jsx("<></>", [] |> of_frag)),
    "pp_jsx() - tag with attributes"
    >: (
      () =>
        _assert_jsx(
          "<Foo #bar .fizz buzz />",
          (
            "Foo" |> of_public |> as_raw_node,
            [
              "bar" |> of_public |> as_raw_node |> of_jsx_id |> as_string,
              ("fizz" |> of_public |> as_raw_node, None)
              |> of_jsx_class
              |> as_string,
              ("buzz" |> of_public |> as_raw_node, None)
              |> of_prop
              |> as_generic(0, 0),
            ],
            [],
          )
          |> of_tag,
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
            "Foo" |> of_public |> as_raw_node,
            [],
            ["bar" |> as_string |> of_text |> as_string],
          )
          |> of_tag,
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
            "Foo" |> of_public |> as_raw_node,
            [],
            [
              (1 |> int_prim, 5 |> int_prim)
              |> of_add_op
              |> as_int
              |> of_inline_expr
              |> as_element,
            ],
          )
          |> of_tag,
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
            "Foo" |> of_public |> as_raw_node,
            [],
            [
              (
                "Bar" |> of_public |> as_raw_node,
                [],
                ["fizzbuzz" |> as_string |> of_text |> as_string],
              )
              |> of_tag
              |> as_element
              |> of_node
              |> as_element,
            ],
          )
          |> of_tag,
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
            "Foo" |> of_public |> as_raw_node,
            [],
            [
              ("Bar" |> of_public |> as_raw_node, [], [])
              |> of_tag
              |> as_element
              |> of_node
              |> as_element,
              nil_prim |> of_inline_expr |> as_nil,
              "Hello, World!" |> as_string |> of_text |> as_string,
            ],
          )
          |> of_tag,
        )
    ),
    "pp_jsx_attr() - property with primitive value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=123",
          ("fizz" |> of_public |> as_raw_node, Some(123 |> int_prim))
          |> of_prop,
        )
    ),
    "pp_jsx_attr() - property with identifier value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=buzz",
          (
            "fizz" |> of_public |> as_raw_node,
            Some(
              "buzz"
              |> of_public
              |> as_generic(0, 0)
              |> of_id
              |> as_generic(0, 0),
            ),
          )
          |> of_prop,
        )
    ),
    "pp_jsx_attr() - property with binary operation value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=(1 + 2)",
          (
            "fizz" |> of_public |> as_raw_node,
            Some((1 |> int_prim, 2 |> int_prim) |> of_add_op |> as_int),
          )
          |> of_prop,
        )
    ),
    "pp_jsx_attr() - property with grouped binary operation value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=(1 + 2)",
          (
            "fizz" |> of_public |> as_raw_node,
            Some(
              (1 |> int_prim, 2 |> int_prim)
              |> of_add_op
              |> as_int
              |> of_group
              |> as_int,
            ),
          )
          |> of_prop,
        )
    ),
    "pp_jsx_attr() - property with negative value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=(-1)",
          (
            "fizz" |> of_public |> as_raw_node,
            Some(1 |> int_prim |> of_neg_op |> as_int),
          )
          |> of_prop,
        )
    ),
    "pp_jsx_attr() - property with boolean value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=true",
          (
            "fizz" |> of_public |> as_raw_node,
            Some(
              true |> bool_prim |> of_group |> as_bool |> of_group |> as_bool,
            ),
          )
          |> of_prop,
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
            "fizz" |> of_public |> as_raw_node,
            Some(
              [
                true |> bool_prim |> of_expr |> as_bool,
                false |> bool_prim |> of_expr |> as_bool,
              ]
              |> of_closure
              |> as_bool,
            ),
          )
          |> of_prop,
        )
    ),
    "pp_jsx_attr() - property with inline JSX value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=<Buzz />",
          (
            "fizz" |> of_public |> as_raw_node,
            Some(
              ("Buzz" |> of_public |> as_raw_node, [], [])
              |> of_tag
              |> as_element
              |> of_jsx
              |> as_element,
            ),
          )
          |> of_prop,
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
            "fizz" |> of_public |> as_raw_node,
            Some(
              (
                "Buzz" |> of_public |> as_raw_node,
                [],
                [
                  ("Foo" |> of_public |> as_raw_node, [], [])
                  |> of_tag
                  |> as_element
                  |> of_node
                  |> as_element,
                ],
              )
              |> of_tag
              |> as_element
              |> of_jsx
              |> as_element,
            ),
          )
          |> of_prop,
        )
    ),
    "pp_jsx_attr() - property with punned value"
    >: (
      () =>
        _assert_jsx_attr(
          "buzz",
          ("buzz" |> of_public |> as_raw_node, None) |> of_prop,
        )
    ),
    "pp_jsx_attr() - dynamic class name"
    >: (
      () =>
        _assert_jsx_attr(
          ".fizz=true",
          ("fizz" |> of_public |> as_raw_node, Some(true |> bool_prim))
          |> of_jsx_class,
        )
    ),
    "pp_jsx_attr() - static class name"
    >: (
      () =>
        _assert_jsx_attr(
          ".fizz",
          ("fizz" |> of_public |> as_raw_node, None) |> of_jsx_class,
        )
    ),
    "pp_jsx_attr() - identifier name"
    >: (
      () =>
        _assert_jsx_attr(
          "#bar",
          "bar" |> of_public |> as_raw_node |> of_jsx_id,
        )
    ),
  ];
