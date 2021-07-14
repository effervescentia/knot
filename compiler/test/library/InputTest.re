open Kore;

let __uchar = Uchar.of_char('z');
let __cursor = Cursor.point(4, 0);
let __input = (__uchar, __cursor);

let _assert_context =
  Alcotest.(
    check(
      testable(
        (pp, cursor) =>
          cursor |> Debug.print_cursor |> Format.pp_print_string(pp),
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
        [(__input, Input.create(__uchar, __cursor))]
        |> Assert.(test_many(char))
    ),
    "value()"
    >: (
      () => [(__uchar, Input.value(__input))] |> Assert.(test_many(uchar))
    ),
    "cursor()"
    >: (
      () =>
        [(__cursor, Input.cursor(__input))] |> Assert.(test_many(cursor))
    ),
  ];
