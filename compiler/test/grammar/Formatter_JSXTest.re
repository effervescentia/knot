open Kore;

module Formatter = Language.Formatter;
module U = Util.ResultUtil;

let _assert_jsx = (expected, actual) =>
  Assert.string(
    expected,
    actual
    |> ~@
         Fmt.root(
           KSX.format(
             Expression.Formatter.attribute_needs_wrapper,
             Expression.format,
           ),
         ),
  );
let _assert_jsx_attr = (expected, actual) =>
  Assert.string(
    expected,
    actual
    |> ~@
         Fmt.root(
           KSX.Formatter.format_attribute(
             Expression.Formatter.attribute_needs_wrapper,
             Expression.format,
           ),
         ),
  );

let suite =
  "Grammar.Formatter | JSX"
  >::: [
    "pp_jsx() - empty tag"
    >: (
      () =>
        _assert_jsx(
          "<Foo />",
          ("Foo" |> U.as_view([], Valid(Nil)), [], [], [])
          |> KSX.of_element_tag,
        )
    ),
    "pp_jsx() - empty component"
    >: (
      () =>
        _assert_jsx(
          "<Foo />",
          ("Foo" |> U.as_view([], Valid(Element)), [], [], [])
          |> KSX.of_component_tag,
        )
    ),
    "pp_jsx() - empty fragment"
    >: (() => _assert_jsx("<></>", [] |> KSX.of_fragment)),
    "pp_jsx() - tag with attributes"
    >: (
      () =>
        _assert_jsx(
          "<Foo bar=123 buzz />",
          (
            "Foo" |> U.as_view([], Valid(Nil)),
            [],
            [
              (U.as_untyped("bar"), 123 |> U.int_prim |> Option.some)
              |> U.as_untyped,
              (U.as_untyped("buzz"), None) |> U.as_untyped,
            ],
            [],
          )
          |> KSX.of_element_tag,
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
            "Foo" |> U.as_view([], Valid(Nil)),
            [],
            [],
            ["bar" |> KSX.Child.of_text |> U.as_untyped],
          )
          |> KSX.of_element_tag,
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
            "Foo" |> U.as_view([], Valid(Nil)),
            [],
            [],
            [
              (1 |> U.int_prim, 5 |> U.int_prim)
              |> Expression.of_add_op
              |> U.as_int
              |> KSX.Child.of_inline
              |> U.as_untyped,
            ],
          )
          |> KSX.of_element_tag,
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
            "Foo" |> U.as_view([], Valid(Nil)),
            [],
            [],
            [
              (
                "Bar" |> U.as_view([], Valid(Nil)),
                [],
                [],
                ["fizzbuzz" |> KSX.Child.of_text |> U.as_untyped],
              )
              |> KSX.of_element_tag
              |> KSX.Child.of_node
              |> U.as_untyped,
            ],
          )
          |> KSX.of_element_tag,
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
            "Foo" |> U.as_view([], Valid(Nil)),
            [],
            [],
            [
              ("Bar" |> U.as_view([], Valid(Nil)), [], [], [])
              |> KSX.of_element_tag
              |> KSX.Child.of_node
              |> U.as_untyped,
              U.nil_prim |> KSX.Child.of_inline |> U.as_untyped,
              "Hello, World!" |> KSX.Child.of_text |> U.as_untyped,
            ],
          )
          |> KSX.of_element_tag,
        )
    ),
    "pp_jsx_attr() - property with primitive value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=123",
          (U.as_untyped("fizz"), Some(123 |> U.int_prim)),
        )
    ),
    "pp_jsx_attr() - property with identifier value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=buzz",
          (
            U.as_untyped("fizz"),
            Some("buzz" |> Expression.of_identifier |> U.as_int),
          ),
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
              (1 |> U.int_prim, 2 |> U.int_prim)
              |> Expression.of_add_op
              |> U.as_int,
            ),
          ),
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
              |> Expression.of_add_op
              |> U.as_int
              |> Expression.of_group
              |> U.as_int,
            ),
          ),
        )
    ),
    "pp_jsx_attr() - property with negative value"
    >: (
      () =>
        _assert_jsx_attr(
          "fizz=(-1)",
          (
            U.as_untyped("fizz"),
            Some(1 |> U.int_prim |> Expression.of_negative_op |> U.as_int),
          ),
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
              |> Expression.of_group
              |> U.as_bool
              |> Expression.of_group
              |> U.as_bool,
            ),
          ),
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
                true |> U.bool_prim |> Statement.of_effect |> U.as_bool,
                false |> U.bool_prim |> Statement.of_effect |> U.as_bool,
              ]
              |> Expression.of_closure
              |> U.as_bool,
            ),
          ),
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
              ("Buzz" |> U.as_view([], Valid(Nil)), [], [], [])
              |> KSX.of_element_tag
              |> Expression.of_ksx
              |> U.as_element,
            ),
          ),
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
                "Buzz" |> U.as_view([], Valid(Nil)),
                [],
                [],
                [
                  ("Foo" |> U.as_view([], Valid(Nil)), [], [], [])
                  |> KSX.of_element_tag
                  |> KSX.Child.of_node
                  |> U.as_untyped,
                ],
              )
              |> KSX.of_element_tag
              |> Expression.of_ksx
              |> U.as_element,
            ),
          ),
        )
    ),
    "pp_jsx_attr() - property with punned value"
    >: (() => _assert_jsx_attr("buzz", (U.as_untyped("buzz"), None))),
  ];
