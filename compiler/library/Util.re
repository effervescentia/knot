let print_sequential = (~separator="", printer, xs) => {
  let rec next =
    fun
    | [x] => printer(x)
    | [x, ...xs] => printer(x) ++ separator ++ next(xs)
    | [] => "";

  next(xs);
};

let print_comma_separated = printer =>
  print_sequential(~separator=", ", printer);

let print_optional = printer =>
  fun
  | Some(x) => printer(x)
  | None => "";

let rec repeat = (s, n) =>
  if (n == 0) {
    "";
  } else {
    s ++ repeat(s, n - 1);
  };
