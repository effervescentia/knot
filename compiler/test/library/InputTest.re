open Kore;

let __uchar = Uchar.of_char('z');
let __cursor = Cursor.point(4, 0);
let __char = (__uchar, __cursor);

let suite =
  "Library.Input"
  >::: [
    "create()"
    >: (
      () =>
        [(__char, Input.create(__uchar, __cursor))]
        |> Assert.(test_many(char))
    ),
    "value()"
    >: (
      () => [(__uchar, Input.value(__char))] |> Assert.(test_many(uchar))
    ),
    "context()"
    >: (
      () =>
        [(__cursor, Input.context(__char))] |> Assert.(test_many(cursor))
    ),
  ];
