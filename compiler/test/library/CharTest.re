open Kore;

module CharAssert =
  Assert.Make({
    type t = Char.t;

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              (
                x =>
                  Print.fmt(
                    "%s@%s",
                    [Char.value(x)] |> String.of_uchars,
                    Char.context(x) |> Cursor.to_string,
                  )
              )
              % Format.pp_print_string(pp),
            (==),
          ),
          "cursor matches",
        )
      );
  });

let __uchar = Uchar.of_char('z');
let __cursor = Cursor.point(4, 0);
let __char = (__uchar, __cursor);

let suite =
  "Library - Char"
  >::: [
    "create()"
    >: (
      () =>
        [(__char, Char.create(__uchar, __cursor))] |> CharAssert.test_many
    ),
    "value()"
    >: (() => [(__uchar, Char.value(__char))] |> Assert.(test_many(uchar))),
    "context()"
    >: (
      () =>
        [(__cursor, Char.context(__char))] |> Assert.(test_many(cursor))
    ),
  ];
