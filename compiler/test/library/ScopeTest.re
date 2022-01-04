open Kore;

let __start_point = Point.create(3, 5);
let __end_point = Point.create(4, 8);
let __range = Range.create(__start_point, __end_point);

let suite =
  "Library.Scope"
  >::: ["zero" >: (() => Assert.range(((0, 0), (0, 0)), Range.zero))];
