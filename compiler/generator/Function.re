open Core;

let args_map = "args";

let rec gen_exprs =
  fun
  | [x] => Expression.generate(x) |> Printf.sprintf("return %s;")
  | [x, ...xs] =>
    (Expression.generate(x) |> Printf.sprintf("%s;")) ++ gen_exprs(xs)
  | [] => "";

let gen_param = index =>
  fun
  | (name, _, default_val) =>
    Printf.sprintf(
      "var %s=%s.arg(arguments,%n,%s%s)",
      name,
      util_map,
      index,
      name,
      switch (default_val) {
      | Some(v) => Expression.generate(v) |> Printf.sprintf(",%s")
      | None => ""
      },
    );

let rec gen_params = params => {
  let rec next = index =>
    index < List.length(params) ?
      (gen_param(index, List.nth(params, index)) |> Printf.sprintf("%s;"))
      ++ next(index + 1) :
      "";

  next(0);
};

let gen_body = (params, exprs) =>
  Printf.sprintf(
    "(%s){%s%s}",
    args_map,
    gen_params(params),
    gen_exprs(exprs),
  );