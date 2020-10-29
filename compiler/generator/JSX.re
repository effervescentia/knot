open Globals;

let createEl = Printf.sprintf("%s.createElement(%s)", jsx_plugin);
let createFrag = Printf.sprintf("%s.createFragment(%s)", jsx_plugin);

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
let rec extract_classes_from_tags =
  fun
  | [] => []
  | [x, ...xs] =>
    switch (x) {
    | ElementClass(name) => [
        Printf.sprintf("$$_class_%s", name),
        ...extract_classes_from_tags(xs),
      ]

    | _ => extract_classes_from_tags(xs)
    };

let rec generate = gen_expression =>
  fun
  | Element(name, tags, props, children) => {
      let all_props =
        switch (extract_classes_from_tags(tags)) {
        | [] => props
        | classes => [
            (
              "className",
              no_ctx(
                Reference(
                  no_ctx(
                    Execution(
                      no_ctx(
                        DotAccess(
                          no_ctx(Variable(style_plugin)),
                          "classes",
                        ),
                      ),
                      List.map(
                        name => no_ctx(Reference(no_ctx(Variable(name)))),
                        classes,
                      ),
                    ),
                  ),
                ),
              ),
            ),
            ...props,
          ]
        };

      switch (all_props, children) {
      | ([], []) => gen_tag(name) |> createEl
      | ([], _) =>
        Printf.sprintf(
          "%s,null,%s",
          gen_tag(name),
          gen_list(generate(gen_expression), children),
        )
        |> createEl
      | (props, _) =>
        Printf.sprintf(
          "%s,%s%s",
          gen_tag(name),
          gen_list(gen_jsx_prop(gen_expression), props)
          |> Printf.sprintf("{%s}"),
          gen_rest(generate(gen_expression), children),
        )
        |> createEl
      };
    }
  | Fragment(children) =>
    gen_list(generate(gen_expression), children) |> createFrag
  | TextNode(s) => s |> gen_string
  | EvalNode((expr, _)) => gen_expression(expr);
