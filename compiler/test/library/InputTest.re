open Kore;

let __uchar = Uchar.of_char('z');
let __cursor = Cursor.point(4, 0);
let __context = Input.{cursor: __cursor};
let __input = (__uchar, __context);

let _assert_context =
  Alcotest.(
    check(
      testable(
        (pp, Input.{cursor}) =>
          cursor |> Cursor.to_string |> Format.pp_print_string(pp),
        (==),
      ),
      "cursor matches",
    )
  );

let suite =
  "Library.Input"
  >::: [
    "create()"
    >: (
      () =>
        [(__input, Input.create(__uchar, __context))]
        |> Assert.(test_many(char))
    ),
    "value()"
    >: (
      () => [(__uchar, Input.value(__input))] |> Assert.(test_many(uchar))
    ),
    "context()"
    >: (
      () =>
        [(__context, Input.context(__input))]
        |> Assert.(test_many(_assert_context))
    ),
    "cursor()"
    >: (
      () =>
        [(__cursor, Input.cursor(__input))] |> Assert.(test_many(cursor))
    ),
  ];
