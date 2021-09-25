open Kore;

let __uchar = Uchar.of_char('z');
let __point = Point.create(4, 0);
let __input = (__uchar, __point);

let suite =
  "Library.Input"
  >::: [
    "create()"
    >: (
      () =>
        [(__input, Input.create(__uchar, __point))]
        |> Assert.(test_many(input))
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
    "join()"
    >: (
      () =>
        [
          (
            Node.Raw.create("foo", Range.create((1, 1), (1, 3))),
            Input.join([
              (Uchar.of_char('f'), Point.create(1, 1)),
              (Uchar.of_char('o'), Point.create(1, 2)),
              (Uchar.of_char('o'), Point.create(1, 3)),
            ]),
          ),
        ]
        |> Assert.(test_many(raw_node(Functional.identity)))
    ),
  ];
