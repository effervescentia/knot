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

let rec flatten = (print: string => unit, column: int) =>
  fun
  | [] => ()

  | [(Nil, _), ...xs] => flatten(print, column, xs)

  | [(String(s), indent), ...xs] =>
    if (String.contains(s, '\n')) {
      flatten(
        print,
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

      if (spaces > 0) {
        String.repeat(spaces, " ") |> print;
      };

      print(s);

      flatten(print, column + spaces + (s |> String.length), xs);
    }

  | [(Append(l, r), indent), ...xs] =>
    flatten(print, column, [(l, indent), (r, indent), ...xs])

  | [(Newline, indent), ...xs] => {
      print("\n");

      flatten(print, 0, xs);
    }

  | [(Indent(x, indent), root_indent), ...xs] =>
    flatten(print, column, [(x, root_indent + indent), ...xs]);

let string = x => String(x);
let indent = (x, y) => Indent(y, x);
let append = ((l, r)) => Append(l, r);
let rec concat = xs =>
  switch (xs) {
  | [] => Nil
  | [x, ...xs] => Append(x, concat(xs))
  };
let newline = xs => Append(concat(xs), Newline);

let to_string = x => {
  let buffer = Buffer.create(100);

  flatten(Buffer.add_string(buffer), 0, [(x, 0)]);

  buffer |> Buffer.contents;
};
