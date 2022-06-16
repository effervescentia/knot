open Kore;

let __uchar = Uchar.of_char('z');
let __point = Point.create(4, 0);
let __input = (__uchar, __point);

let suite =
  "Library.Input"
  >::: [
    "create()"
    >: (() => Assert.input(__input, Input.create(__uchar, __point))),
    "get_value()" >: (() => Assert.uchar(__uchar, Input.get_value(__input))),
    "get_point()" >: (() => Assert.point(__point, Input.get_point(__input))),
    "join()"
    >: (
      () =>
        Assert.untyped_node(
          Fmt.string,
          Node2.untyped("foo", Range.create((1, 1), (1, 3))),
          Input.join([
            (Uchar.of_char('f'), Point.create(1, 1)),
            (Uchar.of_char('o'), Point.create(1, 2)),
            (Uchar.of_char('o'), Point.create(1, 3)),
          ]),
        )
    ),
    "pp()" >: (() => Assert.string("z@4.0", __input |> ~@Input.pp)),
  ];
