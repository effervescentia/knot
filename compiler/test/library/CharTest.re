open Kore;

let __uchar = Uchar.of_char('z');
let __cursor = Cursor.point(4, 0);
let __char = (__uchar, __cursor);

let suite =
  "Library.Char"
  >::: [
    "create()"
    >: (
      () =>
        [(__char, Char.create(__uchar, __cursor))]
        |> Assert.(test_many(char))
    ),
    "value()"
    >: (() => [(__uchar, Char.value(__char))] |> Assert.(test_many(uchar))),
    "context()"
    >: (
      () =>
        [(__cursor, Char.context(__char))] |> Assert.(test_many(cursor))
    ),
  ];
