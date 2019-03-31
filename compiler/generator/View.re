open Core;

let gen_mixin = name =>
  fun
  | State_t(_, props) =>
    Hashtbl.fold(
      (key, _, acc) =>
        acc
        ++ Printf.sprintf("var $%s=%s.%s.%s;", key, props_map, state_map, key),
      props,
      "",
    )

  | _ => "";

let gen_mixins =
  Knot.Util.print_sequential(~separator="", ((name, type_)) =>
    gen_mixin(name, unwrap_type(type_^))
  );

let gen_prop = index =>
  fun
  | (name, _, default_val) =>
    Printf.sprintf(
      "var %s=%s.prop(%s,'%s'%s)",
      name,
      util_map,
      props_map,
      name,
      switch (default_val) {
      | Some((v, _)) => Expression.generate(v) |> Printf.sprintf(",%s")
      | None => ""
      },
    );

let gen_props = props => {
  let rec next = index =>
    index < List.length(props) ?
      (gen_prop(index, List.nth(props, index)) |> Printf.sprintf("%s;"))
      ++ next(index + 1) :
      "";

  next(0);
};

let rec gen_with_hocs =
  fun
  | [(name, type_), ...xs] =>
    switch (unwrap_type(type_^)) {
    | State_t(_) =>
      Printf.sprintf("%s.withState(%s, %s)", jsxGlobal, name)
      % gen_with_hocs(xs)
    | _ => (x => x)
    }
  | [] => (x => x);

let generate = (name, mixins, props, exprs) =>
  Printf.sprintf(
    "function(%s){%s%s%s}",
    props_map,
    gen_mixins(mixins),
    List.map(fst, props) |> gen_props,
    List.map(fst, exprs) |> Function.gen_exprs,
  )
  |> gen_with_hocs(mixins)
  |> Printf.sprintf("var %s=%s;", name);
