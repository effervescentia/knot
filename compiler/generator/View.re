open Core;

let props_map = "$$_props";

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

let generate = (props, exprs) =>
  Printf.sprintf(
    "(%s){%s%s}",
    props_map,
    List.map(fst, props) |> gen_props,
    List.map(fst, exprs) |> Function.gen_exprs,
  );
