open Core;

let rec extract_props_from_tags =
  fun
  | [] => []
  | [x, ...xs] =>
    switch (x) {
    | ElementClass(clazz) => [
        (
          "className",
          Reference(DotAccess(Variable(class_map), Variable(clazz))),
        ),
        ...extract_props_from_tags(xs),
      ]
    | _ => extract_props_from_tags(xs)
    };

let rec generate =
  fun
  | AddExpr(lhs, rhs) => gen_binary("+", lhs, rhs)
  | SubExpr(lhs, rhs) => gen_binary("-", lhs, rhs)
  | MulExpr(lhs, rhs) => gen_binary("*", lhs, rhs)
  | DivExpr(lhs, rhs) => gen_binary("/", lhs, rhs)
  | LTExpr(lhs, rhs) => gen_binary("<", lhs, rhs)
  | LTEExpr(lhs, rhs) => gen_binary("<=", lhs, rhs)
  | GTExpr(lhs, rhs) => gen_binary(">", lhs, rhs)
  | GTEExpr(lhs, rhs) => gen_binary(">=", lhs, rhs)
  | AndExpr(lhs, rhs) => gen_binary("&&", lhs, rhs)
  | OrExpr(lhs, rhs) => gen_binary("||", lhs, rhs)
  | Reference(reference) => gen_reference(reference)
  | JSX(jsx) => gen_jsx(jsx)
  | NumericLit(n) => string_of_int(n)
  | BooleanLit(b) => string_of_bool(b)
  | StringLit(s) => gen_string(s)
and gen_binary = (divider, lhs, rhs) =>
  Printf.sprintf("(%s%s%s)", generate(lhs), divider, generate(rhs))
and gen_reference =
  fun
  | Variable(name) => name
  | DotAccess(lhs, rhs) =>
    Printf.sprintf(
      "%s%s",
      gen_reference(lhs),
      gen_reference(rhs) |> Property.gen_access,
    )
  | Execution(target, args) =>
    Printf.sprintf(
      "%s(%s)",
      gen_reference(target),
      gen_list(generate, args),
    )
and gen_jsx =
  fun
  | Element(name, tags, props, children) =>
    switch (props @ extract_props_from_tags(tags), children) {
    | ([], []) => Printf.sprintf("JSX.createElement(%s)", gen_tag(name))
    | ([], _) =>
      Printf.sprintf(
        "JSX.createElement(%s,null,%s)",
        gen_tag(name),
        gen_list(gen_jsx, children),
      )
    | (ps, _) =>
      Printf.sprintf(
        "JSX.createElement(%s,%s%s)",
        gen_tag(name),
        Knot.Util.print_separated(",", gen_jsx_prop, ps)
        |> Printf.sprintf("{%s}"),
        gen_rest(gen_jsx, children),
      )
    }
  | Fragment(children) =>
    gen_list(gen_jsx, children) |> Printf.sprintf("JSX.createFragment(%s)")
  | TextNode(s) => s |> gen_string
  | EvalNode(expr) => generate(expr)
/**
 * improve this to be based on semantics
 */
and gen_tag = name =>
  switch (name.[0]) {
  | 'a'..'z' => Printf.sprintf("'%s'", name)
  | _ => name
  }
and gen_jsx_prop = ((name, value)) =>
  Printf.sprintf("%s:%s", Property.gen_key(name), generate(value));