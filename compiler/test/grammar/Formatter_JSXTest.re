open Kore;

module Formatter = Language.Formatter;
module U = Util.ResultUtil;

let _assert_jsx = (expected, actual) =>
  Assert.string(
    expected,
    actual |> ~@Fmt.root(KSX.Plugin.pp(KExpression.Plugin.pp)),
  );
let _assert_jsx_attr = (expected, actual) =>
  Assert.string(
    expected,
    actual |> ~@Fmt.root(KSX.Formatter.pp_attr(KExpression.Plugin.pp)),
  );

let suite =
  "Grammar.Formatter | JSX"
  >::: [
    "pp_jsx() - empty tag"
    >: (
      () =>
        _assert_jsx("<Foo />", (U.as_untyped("Foo"), [], []) |> A.of_tag)
    ),
    "pp_jsx() - empty component"
    >: (
      () =>
        _assert_jsx(
          "<Foo />",
          ("Foo" |> U.as_view([], T.Valid(`Element)), [], [])
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
            U.as_untyped("Foo"),
            [
              "bar" |> U.as_untyped |> A.of_jsx_id |> U.as_untyped,
              (U.as_untyped("fizz"), None) |> A.of_jsx_class |> U.as_untyped,
              (U.as_untyped("buzz"), None) |> A.of_prop |> U.as_untyped,
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
          (U.as_untyped("Foo"), [], ["bar" |> A.of_text |> U.as_untyped])
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
            U.as_untyped("Foo"),
            [],
            [
              (1 |> U.int_prim, 5 |> U.int_prim)
              |> A.of_add_op
              |> U.as_int
              |> A.of_inline_expr
              |> U.as_untyped,
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
            U.as_untyped("Foo"),
            [],
            [
              (
                U.as_untyped("Bar"),
                [],
                ["fizzbuzz" |> A.of_text |> U.as_untyped],
              )
              |> A.of_tag
              |> A.of_node
              |> U.as_untyped,
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
            U.as_untyped("Foo"),
            [],
            [
              (U.as_untyped("Bar"), [], [])
              |> A.of_tag
              |> A.of_node
              |> U.as_untyped,
              U.nil_prim |> A.of_inline_expr |> U.as_untyped,
              "Hello, World!" |> A.of_text |> U.as_untyped,
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
          (U.as_untyped("fizz"), Some(123 |> U.int_prim)) |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with identifier value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=buzz",
          (U.as_untyped("fizz"), Some("buzz" |> A.of_id |> U.as_int))
          |> A.of_prop,
        )
    ),
    "pp_jsx_attr() - property with binary operation value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=(1 + 2)",
          (
            U.as_untyped("fizz"),
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
            U.as_untyped("fizz"),
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
            U.as_untyped("fizz"),
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
            U.as_untyped("fizz"),
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
            U.as_untyped("fizz"),
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
            U.as_untyped("fizz"),
            Some(
              (U.as_untyped("Buzz"), [], [])
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
            U.as_untyped("fizz"),
            Some(
              (
                U.as_untyped("Buzz"),
                [],
                [
                  (U.as_untyped("Foo"), [], [])
                  |> A.of_tag
                  |> A.of_node
                  |> U.as_untyped,
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
        _assert_jsx_attr("buzz", (U.as_untyped("buzz"), None) |> A.of_prop)
    ),
    "pp_jsx_attr() - dynamic class name"
    >: (
      () =>
        _assert_jsx_attr(
          ".fizz=true",
          (U.as_untyped("fizz"), Some(true |> U.bool_prim))
          |> A.of_jsx_class,
        )
    ),
    "pp_jsx_attr() - static class name"
    >: (
      () =>
        _assert_jsx_attr(
          ".fizz",
          (U.as_untyped("fizz"), None) |> A.of_jsx_class,
        )
    ),
    "pp_jsx_attr() - identifier name"
    >: (() => _assert_jsx_attr("#bar", "bar" |> U.as_untyped |> A.of_jsx_id)),
  ];
