open Core;

let jsxGlobal = "$$_jsxPlugin.main";
let createEl = Printf.sprintf("%s.createElement(%s)", jsxGlobal);
let createFrag = Printf.sprintf("%s.createFragment(%s)", jsxGlobal);

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
    fst(value) |> gen_expression,
  );

let rec generate = gen_expression =>
  fun
  | Element(name, props, children) =>
    switch (props, children) {
    | ([], []) => gen_tag(name) |> createEl
    | ([], _) =>
      Printf.sprintf(
        "%s,null,%s",
        gen_tag(name),
        gen_list(generate(gen_expression), children),
      )
      |> createEl
    | _ =>
      Printf.sprintf(
        "%s,%s%s",
        gen_tag(name),
        gen_list(gen_jsx_prop(gen_expression), props)
        |> Printf.sprintf("{%s}"),
        gen_rest(generate(gen_expression), children),
      )
      |> createEl
    }
  | Fragment(children) =>
    gen_list(generate(gen_expression), children) |> createFrag
  | TextNode(s) => s |> gen_string
  | EvalNode((expr, _)) => gen_expression(expr);
