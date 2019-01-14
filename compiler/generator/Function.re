open Core;

let args_map = "args";

let rec gen_exprs =
  fun
  | [x] => Expression.generate(x) |> Printf.sprintf("return %s;")
  | [x, ...xs] =>
    (Expression.generate(x) |> Printf.sprintf("%s;")) ++ gen_exprs(xs)
  | [] => "";

let gen_param =
  fun
  | (name, _, Some(default_val)) =>
    Printf.sprintf(
      "var %s='%s' in %s?%s['%s']:%s",
      name,
      name,
      args_map,
      args_map,
      name,
      Expression.generate(default_val),
    )
  | (name, _, None) =>
    Printf.sprintf("var %s=%s['%s']", name, args_map, name);

let gen_body = (params, exprs) =>
  Printf.sprintf(
    "(%s){%s%s}",
    args_map,
    gen_terminated(gen_param, params),
    gen_exprs(exprs),
  );