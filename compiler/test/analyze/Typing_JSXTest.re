open Kore;

module U = Util.RawUtil;
module T = AST.Type;

let _assert_errors_with_ranges =
  Alcotest.(
    check(
      testable(
        Fmt.(
          list((ppf, (err, range)) =>
            pf(ppf, "(%a, %a)", T.pp_error, err, option(Range.pp), range)
          )
        ),
        (==),
      ),
      "type errors match",
    )
  );

let __id = "Foo";

let suite =
  "Analyze.Typing | JSX"
  >::: [
    "inline expression with invalid type"
    >: (
      () =>
        Assert.type_error(
          None,
          KSX.Validator.validate_ksx_primitive_expression(
            Invalid(NotInferrable),
          ),
        )
    ),
    "inline expression with non-function types"
    >: (
      () =>
        [
          T.Valid(`Nil),
          T.Valid(`Boolean),
          T.Valid(`Integer),
          T.Valid(`Float),
          T.Valid(`String),
          T.Valid(`Element),
        ]
        |> List.iter(type_ =>
             Assert.type_error(
               None,
               KSX.Validator.validate_ksx_primitive_expression(type_),
             )
           )
    ),
    "inline expression with function type"
    >: (
      () =>
        Assert.type_error(
          Some(
            InvalidKSXPrimitiveExpression(
              Valid(`Function(([], Valid(`Nil)))),
            ),
          ),
          KSX.Validator.validate_ksx_primitive_expression(
            Valid(`Function(([], Valid(`Nil)))),
          ),
        )
    ),
    "ksx render with non-view type"
    >: (
      () =>
        _assert_errors_with_ranges(
          [
            (
              T.InvalidKSXTag(
                __id,
                T.Valid(`Integer),
                [("foo", T.Valid(`Boolean)), ("bar", T.Valid(`String))],
              ),
              None,
            ),
          ],
          (
            __id,
            T.Valid(`Integer),
            [
              ("foo", T.Valid(`Boolean) |> U.as_untyped),
              ("bar", T.Valid(`String) |> U.as_untyped),
            ],
          )
          |> KSX.Validator.validate_ksx_render(false),
        )
    ),
    "ksx render with invalid attributes"
    >: (
      () => {
        let foo_range = Range.create((10, 2), (12, 3));
        let bar_range = Range.create((2, 3), (4, 5));

        _assert_errors_with_ranges(
          [
            (
              T.InvalidKSXAttribute("bar", T.Valid(`String), T.Valid(`Nil)),
              Some(bar_range),
            ),
            (
              T.InvalidKSXAttribute(
                "foo",
                T.Valid(`Boolean),
                T.Valid(`Integer),
              ),
              Some(foo_range),
            ),
          ],
          (
            __id,
            T.Valid(
              `View((
                [
                  ("foo", (T.Valid(`Boolean), true)),
                  ("bar", (T.Valid(`String), true)),
                ],
                T.Valid(`Element),
              )),
            ),
            [
              ("bar", N.raw(T.Valid(`Nil), bar_range)),
              ("foo", N.raw(T.Valid(`Integer), foo_range)),
            ],
          )
          |> KSX.Validator.validate_ksx_render(false),
        );
      }
    ),
    "ksx render with unexpected attributes"
    >: (
      () => {
        let foo_range = Range.create((10, 2), (12, 3));
        let bar_range = Range.create((2, 3), (4, 5));

        _assert_errors_with_ranges(
          [
            (
              T.UnexpectedKSXAttribute("bar", T.Valid(`Nil)),
              Some(bar_range),
            ),
            (
              T.UnexpectedKSXAttribute("foo", T.Valid(`Integer)),
              Some(foo_range),
            ),
          ],
          (
            __id,
            T.Valid(`View(([], T.Valid(`Element)))),
            [
              ("foo", N.raw(T.Valid(`Integer), foo_range)),
              ("bar", N.raw(T.Valid(`Nil), bar_range)),
            ],
          )
          |> KSX.Validator.validate_ksx_render(false),
        );
      }
    ),
    "ksx render with missing attributes"
    >: (
      () =>
        _assert_errors_with_ranges(
          [
            (
              T.MissingKSXAttributes(
                __id,
                [("bar", T.Valid(`String)), ("foo", T.Valid(`Boolean))],
              ),
              None,
            ),
          ],
          (
            __id,
            T.Valid(
              `View((
                [
                  ("foo", (T.Valid(`Boolean), true)),
                  ("bar", (T.Valid(`String), true)),
                ],
                T.Valid(`Element),
              )),
            ),
            [],
          )
          |> KSX.Validator.validate_ksx_render(false),
        )
    ),
    "ksx render with invalid, unexpected and missing attributes"
    >: (
      () => {
        let foo_range = Range.create((10, 2), (12, 3));
        let bar_range = Range.create((2, 3), (4, 5));

        _assert_errors_with_ranges(
          [
            (
              T.UnexpectedKSXAttribute("bar", T.Valid(`Nil)),
              Some(bar_range),
            ),
            (
              T.InvalidKSXAttribute(
                "foo",
                T.Valid(`Boolean),
                T.Valid(`Integer),
              ),
              Some(foo_range),
            ),
          ],
          (
            __id,
            T.Valid(
              `View((
                [
                  ("foo", (T.Valid(`Boolean), true)),
                  ("fizz", (T.Valid(`Float), true)),
                ],
                T.Valid(`Element),
              )),
            ),
            [
              ("foo", N.raw(T.Valid(`Integer), foo_range)),
              ("bar", N.raw(T.Valid(`Nil), bar_range)),
            ],
          )
          |> KSX.Validator.validate_ksx_render(false),
        );
      }
    ),
    "ksx render with valid attributes"
    >: (
      () =>
        _assert_errors_with_ranges(
          [],
          (
            __id,
            T.Valid(
              `View((
                [
                  ("foo", (T.Valid(`Boolean), true)),
                  ("bar", (T.Valid(`Float), true)),
                ],
                T.Valid(`Element),
              )),
            ),
            [
              ("foo", T.Valid(`Boolean) |> U.as_untyped),
              ("bar", T.Valid(`Float) |> U.as_untyped),
            ],
          )
          |> KSX.Validator.validate_ksx_render(false),
        )
    ),
    "ksx render with optional attributes"
    >: (
      () =>
        _assert_errors_with_ranges(
          [],
          (
            __id,
            T.Valid(
              `View((
                [
                  ("foo", (T.Valid(`Boolean), false)),
                  ("bar", (T.Valid(`Float), false)),
                ],
                T.Valid(`Element),
              )),
            ),
            [("foo", T.Valid(`Boolean) |> U.as_untyped)],
          )
          |> KSX.Validator.validate_ksx_render(false),
        )
    ),
  ];
