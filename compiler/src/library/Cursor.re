type point_t = {
  line: int,
  column: int,
};
type range_t = (point_t, point_t);

/**
 container representing a selection of a source document
 */
type t =
  | Point(point_t)
  | Range(point_t, point_t);

/* static */

let point = (line: int, column: int): t => Point({line, column});

let range = ((line, column): (int, int), (line', column'): (int, int)): t =>
  Range({line, column}, {line: line', column: column'});

let zero = point(0, 0);

/* methods */

/**
 combine two cursors
 */
let join = (x: t, y: t): t =>
  switch (x, y) {
  | _ when x == y => x

  | (Point(start), Point(end_))
  | (Range(start, _), Point(end_))
  | (Point(start), Range(_, end_))
  | (Range(start, _), Range(_, end_)) => Range(start, end_)
  };

let expand =
  fun
  | Range(start, end_) => (start, end_)
  | Point(point) => (point, point);

let is_in_range = ((start, end_): range_t, point: point_t) =>
  Int.contains((start.line, end_.line), point.line)
  && (
    if (start.line == end_.line) {
      Int.contains((start.column, end_.column), point.column);
    } else if (start.line == point.line) {
      point.column >= start.column;
    } else if (end_.line == point.line) {
      point.column <= end_.column;
    } else {
      true;
    }
  );
