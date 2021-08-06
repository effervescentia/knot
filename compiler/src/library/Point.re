type t = (int, int);

/* static */

let create = (line: int, column: int): t => (line, column);

let zero = create(0, 0);

/* getters */

let get_line = ((line, _): t): int => line;
let get_column = ((_, column): t): int => column;

/* methods */

let to_string = ((line, column): t) => Print.fmt("(%d:%d)", line, column);
