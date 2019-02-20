open Core;

/**
 * improve this to be based on semantics
 */
let gen_tag = name =>
  switch (name.[0]) {
  | 'a'..'z' => Printf.sprintf("'%s'", name)
  | _ => name
  };
let gen_jsx_prop = (gen_expression, (name, value)) =>
  Printf.sprintf(
    "%s:%s",
    Property.gen_key(name),
    unwrap(value) |> gen_expression,
  );

let rec generate = gen_expression =>
  fun
  | A_Element(name, props, children) =>
    switch (props, children) {
    | ([], []) => Printf.sprintf("JSX.createElement(%s)", gen_tag(name))
    | ([], _) =>
      Printf.sprintf(
        "JSX.createElement(%s,null,%s)",
        gen_tag(name),
        gen_list(unwrap % generate(gen_expression), children),
      )
    | _ =>
      Printf.sprintf(
        "JSX.createElement(%s,%s%s)",
        gen_tag(name),
        gen_list(gen_jsx_prop(gen_expression), props)
        |> Printf.sprintf("{%s}"),
        gen_rest(unwrap % generate(gen_expression), children),
      )
    }
  | A_Fragment(children) =>
    gen_list(unwrap % generate(gen_expression), children)
    |> Printf.sprintf("JSX.createFragment(%s)")
  | A_TextNode(s) => s |> gen_string
  | A_EvalNode(expr) => unwrap(expr) |> gen_expression;