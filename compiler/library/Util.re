let print_separated = (separator, printer, xs) => {
  let rec next =
    fun
    | [x] => printer(x)
    | [x, ...xs] => printer(x) ++ separator ++ next(xs)
    | [] => "";

  next(xs);
};
