/**
 Pretty printing utilities.
 */

/* add Breakpoint to add supported breakpoints when auto-wrapping long lines? */
type t =
  | Nil
  | String(string)
  | Newline
  | Indent(t, int)
  | Append(t, t);

let rec _flatten = (column: int, x: list((t, int))) =>
  switch (x) {
  | [] => ""

  | [(Nil, _), ...xs] => _flatten(column, xs)

  | [(String(s), indent), ...xs] =>
    if (String.contains(s, '\n')) {
      _flatten(
        column,
        (
          String.split_on_char('\n', s)
          |> List.map(x => (x == "" ? Nil : String(x), indent))
          |> List.intersperse((Newline, indent))
        )
        @ xs,
      );
    } else {
      let spaces = indent - column;

      (spaces > 0 ? String.repeat(spaces, " ") : "")
      ++ s
      ++ _flatten(column + spaces + (s |> String.length), xs);
    }

  | [(Append(l, r), indent), ...xs] =>
    _flatten(column, [(l, indent), (r, indent), ...xs])

  | [(Newline, indent), ...xs] => "\n" ++ _flatten(0, xs)

  | [(Indent(x, indent), root_indent), ...xs] =>
    _flatten(column, [(x, root_indent + indent), ...xs])
  };

let string = x => String(x);
let indent = (x, y) => Indent(y, x);
let append = ((l, r)) => Append(l, r);
let rec concat = xs =>
  switch (xs) {
  | [] => Nil
  | [x, ...xs] => Append(x, concat(xs))
  };
let newline = xs => Append(concat(xs), Newline);

let to_string = x => _flatten(0, [(x, 0)]);
