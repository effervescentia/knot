open Kore;
open AST;
open Reference;
open Pretty;

let __space = string(" ");
let __semicolon = string(";");
let __quotation_mark = string("\"");

let pp_binary_op: Fmt.t(binary_operator_t) =
  ppf =>
    (
      fun
      | LogicalAnd => "&&"
      | LogicalOr => "||"
      | Add => "+"
      | Subtract => "-"
      | Divide => "/"
      | Multiply => "*"
      | LessOrEqual => "<="
      | LessThan => "<"
      | GreaterOrEqual => ">="
      | GreaterThan => ">"
      | Equal => "=="
      | Unequal => "!="
      | Exponent => "^"
    )
    % Fmt.string(ppf);

let pp_unary_op: Fmt.t(unary_operator_t) =
  ppf =>
    (
      fun
      | Not => "!"
      | Positive => "+"
      | Negative => "-"
    )
    % Fmt.string(ppf);

let pp_num: Fmt.t(number_t) =
  ppf =>
    fun
    | Integer(int) => Fmt.int64(ppf, int)
    | Float(float, precision) => Fmt.pf(ppf, "%.*f", precision, float);

let pp_string: Fmt.t('a) =
  (ppf, s) => s |> String.escaped |> Fmt.pf(ppf, "\"%s\"");

let pp_ns: Fmt.t(Namespace.t) = ppf => ~@Namespace.pp % pp_string(ppf);

let pp_prim: Fmt.t(raw_primitive_t) =
  ppf =>
    fun
    | Nil => Fmt.string(ppf, "nil")
    | Boolean(true) => Fmt.string(ppf, "true")
    | Boolean(false) => Fmt.string(ppf, "false")
    | Number(num) => pp_num(ppf, num)
    | String(str) => pp_string(ppf, str);

let rec pp_jsx: Fmt.t(raw_jsx_t) =
  ppf =>
    fun
    | Tag(name, attrs, []) =>
      Fmt.pf(
        ppf,
        "<%a%a />",
        Identifier.pp,
        Node.Raw.get_value(name),
        pp_jsx_attr_list,
        attrs |> List.map(Node.get_value),
      )
    | Tag(name, attrs, children) =>
      Fmt.(
        pf(
          ppf,
          "<%a%a>%a</%a>",
          Identifier.pp,
          Node.Raw.get_value(name),
          pp_jsx_attr_list,
          attrs |> List.map(Node.get_value),
          inner_box(list(~sep=nop, pp_jsx_child)),
          children |> List.map(Node.get_value),
          Identifier.pp,
          Node.Raw.get_value(name),
        )
      )

    | Fragment([]) => Fmt.pf(ppf, "@ <></>")
    | Fragment(children) =>
      Fmt.pf(
        ppf,
        "@ <>%a</>",
        Fmt.(inner_box(list(~sep=nop, pp_jsx_child))),
        children |> List.map(Node.get_value),
      )

and pp_jsx_child: Fmt.t(raw_jsx_child_t) =
  ppf =>
    fun
    | Node(jsx) => jsx |> Node.get_value |> Fmt.pf(ppf, "@ %a", pp_jsx)
    | Text(text) => text |> Node.get_value |> Fmt.pf(ppf, "@ %s")
    | InlineExpression(expr) =>
      expr |> Node.get_value |> Fmt.pf(ppf, "@ {%a}", pp_expression)

and pp_jsx_attr_list: Fmt.t(list(raw_jsx_attribute_t)) =
  ppf =>
    fun
    | [] => Fmt.nop(ppf, ())
    | attrs =>
      attrs
      |> Fmt.list(~sep=Fmt.nop, ppf => Fmt.pf(ppf, " %a", pp_jsx_attr), ppf)

and pp_jsx_attr: Fmt.t(raw_jsx_attribute_t) =
  (ppf, attr) =>
    Fmt.pf(
      ppf,
      "%a%a",
      ppf =>
        fun
        | Class(name, _) =>
          Fmt.pf(ppf, ".%a", Identifier.pp, Node.Raw.get_value(name))
        | ID(name) =>
          Fmt.pf(ppf, "#%a", Identifier.pp, Node.Raw.get_value(name))
        | Property(name, _) =>
          name |> Node.Raw.get_value |> Identifier.pp(ppf),
      attr,
      ppf =>
        fun
        | Class(_, Some(expr))
        | Property(_, Some(expr)) =>
          expr |> Node.get_value |> Fmt.pf(ppf, "=%a", pp_jsx_attr_expr)
        | _ => Fmt.nop(ppf, ()),
      attr,
    )

and pp_jsx_attr_expr: Fmt.t(raw_expression_t) =
  ppf =>
    fun
    | (
        Primitive(_) | Identifier(_) | Group(_) | Closure(_) |
        /* show tags or fragments with no children */
        JSX((Tag(_, _, []) | Fragment([]), _, _))
      ) as expr =>
      pp_expression(ppf, expr)
    | expr => Fmt.pf(ppf, "(%a)", pp_expression, expr)

and pp_expression: Fmt.t(raw_expression_t) =
  ppf =>
    fun
    | Primitive(prim) => prim |> Node.get_value |> pp_prim(ppf)
    | Identifier(name) => name |> Node.get_value |> Identifier.pp(ppf)
    | JSX(jsx) => jsx |> Node.get_value |> pp_jsx(ppf)

    /* collapse parentheses around unary values */
    | Group((
        (Primitive(_) | Identifier(_) | Group(_) | UnaryOp(_) | Closure(_)) as expr,
        _,
        _,
      )) =>
      pp_expression(ppf, expr)
    | Group(expr) =>
      expr |> Node.get_value |> Fmt.pf(ppf, "(%a)", pp_expression)

    | BinaryOp(op, lhs, rhs) =>
      Fmt.pf(
        ppf,
        "%a %a %a",
        pp_expression,
        Node.get_value(lhs),
        pp_binary_op,
        op,
        pp_expression,
        Node.get_value(rhs),
      )

    | UnaryOp(op, expr) =>
      Fmt.pf(
        ppf,
        "%a%a",
        pp_unary_op,
        op,
        pp_expression,
        Node.get_value(expr),
      )

    | Closure([]) => Fmt.string(ppf, "{}")
    | Closure(stmts) =>
      stmts
      |> List.map(Node.get_value)
      |> Fmt.(pf(ppf, "{%a}", inner_box(list(~sep=nop, pp_statement))))

and pp_statement: Fmt.t(raw_statement_t) =
  (ppf, stmt) =>
    switch (stmt) {
    | Variable(name, expr) =>
      Fmt.pf(
        ppf,
        "let %a = %a;",
        Identifier.pp,
        Node.Raw.get_value(name),
        pp_expression,
        Node.get_value(expr),
      )
    | Expression(expr) =>
      expr |> Node.get_value |> Fmt.pf(ppf, "%a;", pp_expression)
    };

let pp_function_body: Fmt.t(raw_expression_t) =
  ppf =>
    fun
    | Closure(_) as expr => pp_expression(ppf, expr)
    | expr => Fmt.pf(ppf, "%a;", pp_expression, expr);

let pp_function_arg: Fmt.t(raw_argument_t) =
  (ppf, {name, default}) =>
    Fmt.pf(
      ppf,
      "%a%a",
      Identifier.pp,
      Node.Raw.get_value(name),
      ppf =>
        fun
        | Some(expr) =>
          expr |> Node.get_value |> Fmt.pf(ppf, " = %a", pp_expression)
        | None => Fmt.nop(ppf, ()),
      default,
    );

let pp_declaration: Fmt.t((Identifier.t, raw_declaration_t)) =
  (ppf, (name, decl)) =>
    switch (decl) {
    | Constant(expr) =>
      Fmt.pf(
        ppf,
        "const %a = %a;\n",
        Identifier.pp,
        name,
        pp_expression,
        Node.get_value(expr),
      )

    | Function([], expr) =>
      Fmt.pf(
        ppf,
        "func %a -> %a\n",
        Identifier.pp,
        name,
        pp_function_body,
        Node.get_value(expr),
      )
    | Function(args, expr) =>
      Fmt.(
        pf(
          ppf,
          "func %a(%a) -> %a\n",
          Identifier.pp,
          name,
          list(
            ~sep=(ppf, ()) => Fmt.string(ppf, ", "),
            ppf => Fmt.pf(ppf, " = %a", pp_function_arg),
          ),
          args |> List.map(Node.get_value),
          pp_function_body,
          Node.get_value(expr),
        )
      )
    };

type import_spec_t = (
  Namespace.t,
  option(Identifier.t),
  list((Identifier.t, option(untyped_identifier_t))),
);

let pp_named_import: Fmt.t((Identifier.t, option(untyped_identifier_t))) =
  ppf =>
    fun
    | (id, Some(label)) =>
      Fmt.pf(
        ppf,
        "%a as %a",
        Identifier.pp,
        id,
        Identifier.pp,
        Node.Raw.get_value(label),
      )
    | (id, None) => Identifier.pp(ppf, id);

let pp_import: Fmt.t(import_spec_t) =
  (ppf, (namespace, main_import, named_imports)) =>
    Fmt.pf(
      ppf,
      "import%a%a%a from %a;\n",
      ppf =>
        fun
        | Some(id) => Fmt.pf(ppf, " %a", Identifier.pp, id)
        | None => Fmt.nop(ppf, ()),
      main_import,
      ppf =>
        fun
        | (None, _)
        | (_, []) => Fmt.nop(ppf, ())
        | _ => Fmt.string(ppf, ", "),
      (main_import, named_imports),
      ppf =>
        fun
        | [] => Fmt.nop(ppf, ())
        | imports =>
          Fmt.pf(
            ppf,
            "{ %a }",
            Fmt.list(
              ~sep=(ppf, ()) => Fmt.string(ppf, ", "),
              pp_named_import,
            ),
            imports,
          ),
      named_imports,
      Namespace.pp,
      namespace,
    );

let pp_all_imports: Fmt.t((list(import_spec_t), list(import_spec_t))) =
  (ppf, (internal_imports, external_imports)) =>
    [external_imports, internal_imports]
    |> Fmt.list(
         ~sep=(ppf, ()) => Fmt.string(ppf, "\n"),
         Fmt.list(~sep=Fmt.nop, pp_import),
         ppf,
       );

let pp_declaration_list: Fmt.t(list((Identifier.t, raw_declaration_t))) =
  ppf => {
    let rec loop =
      fun
      | [] => Fmt.nop(ppf, ())

      /* do not add newline after the last statement */
      | [decl] => pp_declaration(ppf, decl)

      /* handle constant clustering logic */
      | [(_, Constant(_)) as decl, ...xs] =>
        switch (xs) {
        /* no more statements, loop to return */
        | []
        /* followed by a constant, do not add newline */
        | [(_, Constant(_)), ..._] =>
          pp_declaration(ppf, decl);

          loop(xs);

        /* followed by other declarations, add a newline */
        | _ =>
          Fmt.pf(ppf, "%a\n", pp_declaration, decl);

          loop(xs);
        }

      /* not a constant, add a newline */
      | [decl, ...xs] => Fmt.pf(ppf, "%a\n", pp_declaration, decl);

    loop;
  };

let extract_imports = (program: program_t) =>
  program
  |> List.filter_map(
       Node.Raw.get_value
       % (
         fun
         | Import(namespace, imports) => Some((namespace, imports))
         | _ => None
       ),
     )
  |> List.partition(
       Namespace.(
         fun
         | (Internal(_), _) => true
         | _ => false
       ),
     )
  |> Tuple.map2(
       List.sort((l, r) =>
         (l, r)
         |> Tuple.map2(
              fst
              % Namespace.(
                  fun
                  | Internal(name)
                  | External(name) => name
                ),
            )
         |> Tuple.join2(String.compare)
       )
       % List.map(((namespace, imports)) => {
           let (main_import, named_imports) =
             imports
             |> List.fold_left(
                  ((m, n)) =>
                    Node.Raw.get_value
                    % (
                      fun
                      | MainImport(id) => (Some(Node.Raw.get_value(id)), n)
                      | NamedImport(id, label) => (
                          m,
                          [(Node.Raw.get_value(id), label), ...n],
                        )
                    ),
                  (None, []),
                );

           (namespace, main_import, named_imports);
         }),
     );

let extract_declarations = (program: program_t) =>
  program
  |> List.filter_map(
       Node.Raw.get_value
       % (
         fun
         | Declaration(MainExport(name) | NamedExport(name), decl) =>
           Some((Node.Raw.get_value(name), Node.get_value(decl)))
         | _ => None
       ),
     );

let format: Fmt.t(program_t) =
  (ppf, program) => {
    let imports = extract_imports(program);
    let declarations = extract_declarations(program);

    Fmt.pf(
      ppf,
      "%a%a%a",
      pp_all_imports,
      imports,
      ppf =>
        fun
        | ([], []) => Fmt.nop(ppf, ())
        | _ => Fmt.string(ppf, "\n"),
      (fst(imports) @ snd(imports), declarations),
      pp_declaration_list,
      declarations,
    );
  };
