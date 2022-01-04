open Kore;

let suite =
  "Library.Int"
  >::: [
    "max_of()" >: (() => Assert.int(3, Int.max_of([2, 3, 1]))),
    "contains() - too low" >: (() => Assert.false_(Int.contains((2, 4), 1))),
    "contains() - too high"
    >: (() => Assert.false_(Int.contains((2, 4), 5))),
    "contains() - lower bound"
    >: (() => Assert.true_(Int.contains((2, 4), 2))),
    "contains() - between bounds"
    >: (() => Assert.true_(Int.contains((2, 4), 3))),
    "contains() - upper bound"
    >: (() => Assert.true_(Int.contains((2, 4), 4))),
  ];
