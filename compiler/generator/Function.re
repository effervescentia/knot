open Core;

let gen_expr =
  fun
  | ExpressionStatement((x, _)) =>
    Expression.generate(x) |> Printf.sprintf("%s;")
  | VariableDeclaration(name, (x, _)) =>
    Expression.generate(x) |> Printf.sprintf("var %s=%s;", name);

let rec gen_exprs =
  fun
  | [x] =>
    switch (x) {
    | VariableDeclaration(_) => gen_expr(x) |> Printf.sprintf("%sreturn;")
    | ExpressionStatement(_) => gen_expr(x) |> Printf.sprintf("return %s")
    }
  | [x, ...xs] => gen_expr(x) ++ gen_exprs(xs)
  | [] => "";

let gen_param = index =>
  fun
  | (name, _, default_val) =>
    Printf.sprintf(
      "var %s=%s.arg(arguments,%n,'%s'%s)",
      name,
      util_map,
      index,
      name,
      switch (default_val) {
      | Some((v, _)) => Expression.generate(v) |> Printf.sprintf(",%s")
      | None => ""
      },
    );

let gen_params = params => {
  let rec next = index =>
    index < List.length(params) ?
      (gen_param(index, List.nth(params, index)) |> Printf.sprintf("%s;"))
      ++ next(index + 1) :
      "";

  next(0);
};

let gen_body = (params, exprs) =>
  Printf.sprintf(
    "(){%s%s}",
    List.map(fst, params) |> gen_params,
    List.map(fst, exprs) |> gen_exprs,
  );
