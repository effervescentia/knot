open Globals;

let gen_expr = generate_expr =>
  fun
  | ExpressionStatement((x, _)) => generate_expr(x) |> Printf.sprintf("%s;")
  | VariableDeclaration(name, (x, _)) =>
    generate_expr(x) |> Printf.sprintf("var %s=%s;", name)
  | VariableAssignment(refr, (x, _)) =>
    Printf.sprintf(
      "%s=%s;",
      Reference.generate(generate_expr, fst(refr)),
      generate_expr(x),
    );

let rec gen_exprs = generate_expr =>
  fun
  | [x] =>
    switch (x) {
    | VariableDeclaration(_)
    | VariableAssignment(_) =>
      gen_expr(generate_expr, x) |> Printf.sprintf("%sreturn;")
    | ExpressionStatement(_) =>
      gen_expr(generate_expr, x) |> Printf.sprintf("return %s")
    }
  | [x, ...xs] => gen_expr(generate_expr, x) ++ gen_exprs(generate_expr, xs)
  | [] => "";

let gen_param = (generate_expr, index) =>
  fun
  | (name, _, default_val) =>
    Printf.sprintf(
      "var %s=%s.arg(arguments,%n,'%s'%s)",
      name,
      platform_plugin,
      index,
      name,
      Knot.Print.optional(
        fst % generate_expr % Printf.sprintf(",%s"),
        default_val,
      ),
    );

let gen_params = (generate_expr, params) => {
  let rec next = index =>
    index < List.length(params)
      ? (
          gen_param(generate_expr, index, List.nth(params, index))
          |> Printf.sprintf("%s;")
        )
        ++ next(index + 1)
      : "";

  next(0);
};

let gen_body = (generate_expr, params, exprs) =>
  Printf.sprintf(
    "(){%s%s}",
    List.map(fst, params) |> gen_params(generate_expr),
    List.map(fst, exprs) |> gen_exprs(generate_expr),
  );
