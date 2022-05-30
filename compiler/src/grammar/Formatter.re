open Kore;
open FormatterUtils;

module Namespace = Reference.Namespace;
module Identifier = Reference.Identifier;

let __default_margin = 120;

let pp_binary_op: Fmt.t(A.binary_t) =
  ppf =>
    (
      fun
      | A.LogicalAnd => "&&"
      | A.LogicalOr => "||"
      | A.Add => "+"
      | A.Subtract => "-"
      | A.Divide => "/"
      | A.Multiply => "*"
      | A.LessOrEqual => "<="
      | A.LessThan => "<"
      | A.GreaterOrEqual => ">="
      | A.GreaterThan => ">"
      | A.Equal => "=="
      | A.Unequal => "!="
      | A.Exponent => "^"
    )
    % Fmt.string(ppf);

let pp_unary_op: Fmt.t(A.unary_t) =
  ppf =>
    (
      fun
      | A.Not => "!"
      | A.Positive => "+"
      | A.Negative => "-"
    )
    % Fmt.string(ppf);

let pp_num: Fmt.t(A.number_t) =
  ppf =>
    fun
    | Integer(int) => Fmt.int64(ppf, int)
    | Float(float, precision) => Fmt.pf(ppf, "%.*f", precision, float);

let pp_string: Fmt.t('a) =
  (ppf, s) => s |> String.escaped |> Fmt.pf(ppf, "\"%s\"");

let pp_ns: Fmt.t(Namespace.t) = ppf => ~@Namespace.pp % pp_string(ppf);

let pp_prim: Fmt.t(A.primitive_t) =
  ppf =>
    fun
    | Nil => Fmt.string(ppf, "nil")
    | Boolean(true) => Fmt.string(ppf, "true")
    | Boolean(false) => Fmt.string(ppf, "false")
    | Number(num) => pp_num(ppf, num)
    | String(str) => pp_string(ppf, str);

let rec pp_jsx: Fmt.t(A.jsx_t) =
  ppf =>
    fun
    | Tag(name, attrs, []) =>
      Fmt.pf(
        ppf,
        "@[<h><%a%a@ />@]",
        Identifier.pp,
        NR.get_value(name),
        pp_jsx_attr_list,
        attrs |> List.map(NR.get_value),
      )
    | Tag(name, attrs, children) =>
      Fmt.(
        pf(
          ppf,
          "@[<h><%a%a>@]%a</%a>",
          Identifier.pp,
          NR.get_value(name),
          pp_jsx_attr_list,
          attrs |> List.map(NR.get_value),
          block(~layout=Vertical, ~sep=Sep.trailing_newline, pp_jsx_child),
          children |> List.map(NR.get_value),
          Identifier.pp,
          NR.get_value(name),
        )
      )

    | Fragment([]) => Fmt.pf(ppf, "<></>")
    | Fragment(children) =>
      children
      |> List.map(NR.get_value)
      |> Fmt.(
           collection(
             ~layout=Vertical,
             ~sep=Sep.trailing_newline,
             any("<>"),
             any("</>"),
             pp_jsx_child,
             ppf,
           )
         )

and pp_jsx_child: Fmt.t(A.raw_jsx_child_t) =
  ppf =>
    fun
    | Node(jsx) => jsx |> Fmt.pf(ppf, "%a", pp_jsx)
    | Text(text) => text |> Fmt.string(ppf)
    | InlineExpression(expr) =>
      expr |> N.get_value |> Fmt.pf(ppf, "{%a}", pp_expression)

and pp_jsx_attr_list: Fmt.t(list(A.raw_jsx_attribute_t)) =
  ppf =>
    fun
    | [] => Fmt.nop(ppf, ())
    | attrs =>
      attrs
      |> Fmt.(list(~sep=Sep.nop, ppf => pf(ppf, "@ %a", pp_jsx_attr), ppf))

and pp_jsx_attr: Fmt.t(A.raw_jsx_attribute_t) =
  (ppf, attr) =>
    Fmt.(
      pf(
        ppf,
        "%a%a",
        ppf =>
          fun
          | A.Class(name, _) =>
            pf(ppf, ".%a", Identifier.pp, NR.get_value(name))
          | A.ID(name) => pf(ppf, "#%a", Identifier.pp, NR.get_value(name))
          | A.Property(name, _) =>
            name |> NR.get_value |> Identifier.pp(ppf),
        attr,
        ppf =>
          fun
          | A.Class(_, Some(expr))
          | A.Property(_, Some(expr)) =>
            expr |> N.get_value |> pf(ppf, "=%a", pp_jsx_attr_expr)
          | _ => nop(ppf, ()),
        attr,
      )
    )

and pp_jsx_attr_expr: Fmt.t(A.raw_expression_t) =
  ppf =>
    fun
    | (
        Primitive(_) | Identifier(_) | Group(_) | Closure(_) |
        /* show tags or fragments with no children */
        JSX(Tag(_, _, []) | Fragment([]))
      ) as expr =>
      pp_expression(ppf, expr)
    | expr => Fmt.pf(ppf, "(%a)", pp_expression, expr)

and pp_expression: Fmt.t(A.raw_expression_t) =
  ppf =>
    fun
    | Primitive(prim) => prim |> pp_prim(ppf)
    | Identifier(name) => name |> Identifier.pp(ppf)
    | JSX(jsx) => jsx |> pp_jsx(ppf)

    /* collapse parentheses around unary values */
    | Group((
        (Primitive(_) | Identifier(_) | Group(_) | UnaryOp(_) | Closure(_)) as expr,
        _,
        _,
      )) =>
      pp_expression(ppf, expr)
    | Group(expr) =>
      expr |> N.get_value |> Fmt.pf(ppf, "(%a)", pp_expression)

    | BinaryOp(op, lhs, rhs) =>
      Fmt.pf(
        ppf,
        "%a %a %a",
        pp_expression,
        N.get_value(lhs),
        pp_binary_op,
        op,
        pp_expression,
        N.get_value(rhs),
      )

    | UnaryOp(op, expr) =>
      Fmt.pf(ppf, "%a%a", pp_unary_op, op, pp_expression, N.get_value(expr))

    | Closure([]) => Fmt.string(ppf, "{}")
    | Closure(stmts) =>
      stmts |> List.map(N.get_value) |> Fmt.(closure(pp_statement, ppf))

    | DotAccess(expr, prop) =>
      Fmt.pf(
        ppf,
        "%a.%s",
        pp_expression,
        N.get_value(expr),
        NR.get_value(prop),
      )

    | FunctionCall(expr, args) =>
      Fmt.(
        pf(
          ppf,
          "%a@[<hv>(%a)@]",
          pp_expression,
          N.get_value(expr),
          list(~sep=Sep.trailing_comma, pp_expression),
          args |> List.map(N.get_value),
        )
      )

and pp_statement: Fmt.t(A.raw_statement_t) =
  (ppf, stmt) =>
    switch (stmt) {
    | Variable(name, expr) =>
      Fmt.pf(
        ppf,
        "let %a = %a;",
        Identifier.pp,
        NR.get_value(name),
        pp_expression,
        N.get_value(expr),
      )
    | Expression(expr) =>
      expr |> N.get_value |> Fmt.pf(ppf, "%a;", pp_expression)
    };

let pp_function_body: Fmt.t(A.raw_expression_t) =
  ppf =>
    fun
    | Closure(_) as expr => pp_expression(ppf, expr)
    | expr => Fmt.pf(ppf, "%a;", pp_expression, expr);

let pp_function_arg: Fmt.t(A.raw_argument_t) =
  (ppf, {name, default}) =>
    Fmt.pf(
      ppf,
      "%a%a",
      Identifier.pp,
      NR.get_value(name),
      ppf =>
        fun
        | Some(expr) =>
          expr |> N.get_value |> Fmt.pf(ppf, " = %a", pp_expression)
        | None => Fmt.nop(ppf, ()),
      default,
    );

let pp_declaration: Fmt.t((Identifier.t, A.raw_declaration_t)) =
  (ppf, (name, decl)) =>
    switch (decl) {
    | Constant(expr) =>
      Fmt.pf(
        ppf,
        "const %a = %a;",
        Identifier.pp,
        name,
        pp_expression,
        N.get_value(expr),
      )

    | Function([], expr) =>
      Fmt.pf(
        ppf,
        "@[<v>func %a -> %a@]",
        Identifier.pp,
        name,
        pp_function_body,
        N.get_value(expr),
      )
    | Function(args, expr) =>
      Fmt.(
        pf(
          ppf,
          "@[<v>func @[<h>%a(%a)@] -> %a@]",
          Identifier.pp,
          name,
          list(~sep=Sep.trailing_comma, pp_function_arg),
          args |> List.map(N.get_value),
          pp_function_body,
          N.get_value(expr),
        )
      )

    | View([], expr) =>
      Fmt.(
        pf(
          ppf,
          "@[<v>view %a -> %a@]",
          Identifier.pp,
          name,
          pp_function_body,
          N.get_value(expr),
        )
      )
    | View(props, expr) =>
      Fmt.(
        pf(
          ppf,
          "@[<v>view @[<h>%a(%a)@] -> %a@]",
          Identifier.pp,
          name,
          list(~sep=Sep.trailing_comma, ppf => pp_function_arg(ppf)),
          props |> List.map(N.get_value),
          pp_function_body,
          N.get_value(expr),
        )
      )
    };

let pp_declaration_list: Fmt.t(list((Identifier.t, A.raw_declaration_t))) =
  ppf => {
    let rec loop =
      fun
      | [] => Fmt.nop(ppf, ())

      /* do not add newline after the last statement */
      | [decl] => pp_declaration(ppf, decl)

      /* handle constant clustering logic, separate with newlines */
      | [(_, A.Constant(_)) as decl, ...[(_, A.Constant(_)), ..._] as xs] => {
          pp_declaration(ppf, decl);
          Fmt.cut(ppf, ());

          loop(xs);
        }

      /* followed by declarations that are not constants, add a full line break */
      | [decl, ...xs] => {
          pp_declaration(ppf, decl);
          Fmt.cut(ppf, ());
          Fmt.cut(ppf, ());

          loop(xs);
        };

    loop;
  };

type import_spec_t = (
  Namespace.t,
  option(Identifier.t),
  list((Identifier.t, option(A.identifier_t))),
);

let pp_named_import: Fmt.t((Identifier.t, option(A.identifier_t))) =
  ppf =>
    fun
    | (id, Some(label)) =>
      Fmt.pf(
        ppf,
        "%a as %a",
        Identifier.pp,
        id,
        Identifier.pp,
        NR.get_value(label),
      )
    | (id, None) => Identifier.pp(ppf, id);

let pp_named_import_list:
  Fmt.t(list((Identifier.t, option(A.identifier_t)))) =
  (ppf, imports) => Fmt.(destruct(pp_named_import, ppf, imports));

let pp_main_import: Fmt.t(option(Identifier.t)) =
  ppf =>
    fun
    | Some(id) => Identifier.pp(ppf, id)
    | None => Fmt.nop(ppf, ());

let _pp_import_stmt = (pp_targets: Fmt.t('a)): Fmt.t(('a, Namespace.t)) =>
  Fmt.(
    hvbox((ppf, (targets, namespace)) =>
      pf(ppf, "import %a from %a;", pp_targets, targets, pp_ns, namespace)
    )
  );

let pp_import: Fmt.t(import_spec_t) =
  ppf =>
    fun
    | (namespace, None, []) => Fmt.nop(ppf, ())

    | (namespace, Some(main_import), []) =>
      _pp_import_stmt(Identifier.pp, ppf, (main_import, namespace))

    | (namespace, None, named_imports) =>
      _pp_import_stmt(pp_named_import_list, ppf, (named_imports, namespace))

    | (namespace, Some(main_import), named_imports) =>
      _pp_import_stmt(
        (ppf, (main, named)) =>
          Fmt.(
            pf(
              ppf,
              "%a, %a",
              Identifier.pp,
              main,
              pp_named_import_list,
              named,
            )
          ),
        ppf,
        ((main_import, named_imports), namespace),
      );

let pp_import_list: Fmt.t(list(import_spec_t)) =
  ppf => Fmt.(list(~layout=Vertical, ~sep=Sep.newline, pp_import, ppf));

let pp_all_imports: Fmt.t((list(import_spec_t), list(import_spec_t))) =
  ppf =>
    fun
    | ([], []) => Fmt.nop(ppf, ())

    | (only_imports, [])
    | ([], only_imports) => Fmt.(pp_import_list(ppf, only_imports))

    | (internal_imports, external_imports) =>
      [external_imports, internal_imports]
      |> Fmt.(
           list(
             ~layout=Vertical,
             ~sep=Sep.double_newline,
             pp_import_list,
             ppf,
           )
         );

let format = (~margin=__default_margin): Fmt.t(A.program_t) =>
  (ppf, program) => {
    let orig_margin = Format.get_margin();
    Format.set_margin(margin);

    program
    |> Tuple.split2(extract_imports, extract_declarations)
    |> Fmt.(
         page(
           (ppf, (imports, declarations)) =>
             switch (imports, declarations) {
             | (([], []), []) => Fmt.nop(ppf, ())

             | (_, []) => pp_all_imports(ppf, imports)

             | (([], []), _) => pp_declaration_list(ppf, declarations)

             | _ =>
               pf(
                 ppf,
                 "%a@,@,%a",
                 pp_all_imports,
                 imports,
                 pp_declaration_list,
                 declarations,
               )
             },
           ppf,
         )
       );

    Format.set_margin(orig_margin);
  };
