open Kore;

let __point = Point.create(3, 5);

let suite =
  "Library.Point"
  >::: [
    "zero" >: (() => Assert.point((0, 0), Point.zero)),
    "get_line()" >: (() => Assert.int(3, Point.get_line(__point))),
    "get_column()" >: (() => Assert.int(5, Point.get_column(__point))),
    "compare() - no difference"
    >: (() => Assert.int(0, Point.compare(__point, __point))),
    "compare() - lhs column greater than rhs"
    >: (() => Assert.int(1, Point.compare(Point.create(3, 6), __point))),
    "compare() - rhs column greater than rhs"
    >: (() => Assert.int(-1, Point.compare(__point, Point.create(3, 6)))),
    "compare() - lhs row greater than rhs"
    >: (() => Assert.int(1, Point.compare(__point, Point.create(2, 10)))),
    "compare() - rhs row greater than rhs"
    >: (() => Assert.int(-1, Point.compare(Point.create(2, 10), __point))),
    "max() - no difference"
    >: (() => Assert.point(__point, Point.max(__point, __point))),
    "max() - lhs column greater than rhs"
    >: (() => Assert.point(__point, Point.max(__point, Point.create(3, 4)))),
    "max() - rhs column greater than rhs"
    >: (() => Assert.point(__point, Point.max(Point.create(3, 4), __point))),
    "max() - lhs row greater than rhs"
    >: (
      () => Assert.point(__point, Point.max(__point, Point.create(2, 10)))
    ),
    "max() - rhs row greater than rhs"
    >: (
      () => Assert.point(__point, Point.max(Point.create(2, 10), __point))
    ),
    "min() - no difference"
    >: (() => Assert.point(__point, Point.min(__point, __point))),
    "min() - lhs column lesser than rhs"
    >: (() => Assert.point(__point, Point.min(__point, Point.create(3, 6)))),
    "min() - rhs column lesser than rhs"
    >: (() => Assert.point(__point, Point.min(Point.create(3, 6), __point))),
    "min() - lhs row lesser than rhs"
    >: (
      () => Assert.point(__point, Point.min(Point.create(4, 10), __point))
    ),
    "min() - rhs row lesser than rhs"
    >: (
      () => Assert.point(__point, Point.min(__point, Point.create(4, 10)))
    ),
    "pp()" >: (() => Assert.string("3.5", __point |> ~@Point.pp)),
  ];
