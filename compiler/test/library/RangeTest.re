open Kore;

let __start_point = Point.create(3, 5);
let __end_point = Point.create(4, 8);
let __range = Range.create(__start_point, __end_point);

let suite =
  "Library.Range"
  >::: [
    "zero" >: (() => Assert.range(((0, 0), (0, 0)), Range.zero)),
    "get_start()"
    >: (() => Assert.point(__start_point, Range.get_start(__range))),
    "get_end()" >: (() => Assert.point(__end_point, Range.get_end(__range))),
    "join() - in order"
    >: (
      () =>
        Assert.range(
          Range.create(__start_point, (8, 11)),
          Range.join(__range, ((5, 1), (8, 11))),
        )
    ),
    "join() - out of order"
    >: (
      () =>
        Assert.range(
          Range.create(__start_point, (8, 11)),
          Range.join(((5, 1), (8, 11)), __range),
        )
    ),
    "contains_point() - point is within range"
    >: (
      () => Assert.true_(Range.contains_point(Point.create(3, 10), __range))
    ),
    "contains_point() - point is first in range"
    >: (() => Assert.true_(Range.contains_point(__start_point, __range))),
    "contains_point() - point is last in range"
    >: (() => Assert.true_(Range.contains_point(__end_point, __range))),
    "contains_point() - point is below range"
    >: (
      () => Assert.false_(Range.contains_point(Point.create(2, 0), __range))
    ),
    "contains_point() - point is above range"
    >: (
      () => Assert.false_(Range.contains_point(Point.create(5, 0), __range))
    ),
    "contains() - range contains itself"
    >: (() => Assert.true_(Range.contains(__range, __range))),
    "contains() - below target range"
    >: (() => Assert.false_(Range.contains(((1, 1), (2, 11)), __range))),
    "contains() - starting point is below target range"
    >: (() => Assert.false_(Range.contains(((1, 1), (4, 0)), __range))),
    "contains() - starting point is below, and end point is above, target range"
    >: (() => Assert.false_(Range.contains(((1, 1), (8, 11)), __range))),
    "contains() - end point is above target range"
    >: (() => Assert.false_(Range.contains(((5, 1), (8, 11)), __range))),
    "contains() - end point is above target range"
    >: (() => Assert.false_(Range.contains(((5, 1), (8, 11)), __range))),
    "contains() - extends beyond end of target range"
    >: (
      () =>
        Assert.false_(Range.contains((__start_point, (8, 11)), __range))
    ),
    "contains() - extends beyond start of target range"
    >: (
      () => Assert.false_(Range.contains(((5, 1), __end_point), __range))
    ),
    "pp() - multiple lines"
    >: (() => Assert.string("3.5-4.8", __range |> ~@Range.pp)),
    "pp() - single lines"
    >: (
      () =>
        Assert.string(
          "3.1-3.5",
          Range.create(Point.create(3, 1), __start_point) |> ~@Range.pp,
        )
    ),
    "pp() - single character"
    >: (
      () =>
        Assert.string(
          "3.5",
          Range.create(__start_point, __start_point) |> ~@Range.pp,
        )
    ),
  ];
