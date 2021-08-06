open Kore;

let __uchar = Uchar.of_char('z');
let __point = Point.create(4, 0);
let __input = (__uchar, __point);

let _assert_context =
  Alcotest.(
    check(
      testable(pp => Point.to_string % Format.pp_print_string(pp), (==)),
      "cursor matches",
    )
  );

let suite =
  "Library.Input"
  >::: [
    "create()"
    >: (
      () =>
        [(__input, Input.create(__uchar, __point))]
        |> Assert.(test_many(char))
    ),
    "get_value()"
    >: (
      () =>
        [(__uchar, Input.get_value(__input))] |> Assert.(test_many(uchar))
    ),
    "get_point()"
    >: (
      () =>
        [(__point, Input.get_point(__input))] |> Assert.(test_many(point))
    ),
  ];
