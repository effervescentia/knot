type point_t = {
  line: int,
  column: int,
};

type t =
  | Point(point_t)
  | Range(point_t, point_t);

let point = (line: int, column: int): t => Point({line, column});

let zero = point(0, 0);

let range = (x: t, y: t): t =>
  switch (x, y) {
  | _ when x == y => x

  | (Point(start), Point(end_))
  | (Range(start, _), Point(end_))
  | (Point(start), Range(_, end_))
  | (Range(start, _), Range(_, end_)) => Range(start, end_)
  };

let to_string: t => string =
  fun
  | Point({line, column}) => Print.fmt("[%d:%d]", line, column)
  | Range(start, end_) =>
    Print.fmt(
      "[%d:%d - %d:%d]",
      start.line,
      start.column,
      end_.line,
      end_.column,
    );
