open Knot.Kore;
open AST;

let rec pp_ksx: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.jsx_t) =
  (pp_expression, ppf) =>
    fun
    | Tag((name, _), attrs, [])
    | Component((name, _), attrs, []) =>
      Fmt.pf(
        ppf,
        "@[<h><%s%a@ />@]",
        name,
        pp_attr_list(pp_expression),
        attrs |> List.map(fst),
      )

    | Tag((name, _), attrs, children)
    | Component((name, _), attrs, children) =>
      Fmt.(
        pf(
          ppf,
          "@[<h><%s%a>@]%a</%s>",
          name,
          pp_attr_list(pp_expression),
          attrs |> List.map(fst),
          block(
            ~layout=Vertical,
            ~sep=Sep.trailing_newline,
            pp_child(pp_expression),
          ),
          children |> List.map(fst),
          name,
        )
      )

    | Fragment([]) => Fmt.pf(ppf, "<></>")
    | Fragment(children) =>
      children
      |> List.map(fst)
      |> Fmt.(
           collection(
             ~layout=Vertical,
             ~sep=Sep.trailing_newline,
             any("<>"),
             any("</>"),
             pp_child(pp_expression),
             ppf,
           )
         )

and pp_child: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_jsx_child_t) =
  (pp_expression, ppf) =>
    fun
    | Node(jsx) => jsx |> Fmt.pf(ppf, "%a", pp_ksx(pp_expression))
    | Text(text) => text |> Fmt.string(ppf)
    | InlineExpression((expr, _)) => Fmt.pf(ppf, "{%a}", pp_expression, expr)

and pp_attr_list:
  Fmt.t(Result.raw_expression_t) => Fmt.t(list(Result.raw_jsx_attribute_t)) =
  (pp_expression, ppf) =>
    fun
    | [] => Fmt.nop(ppf, ())
    | attrs =>
      attrs
      |> Fmt.(
           list(
             ~sep=Sep.nop,
             ppf => pf(ppf, "@ %a", pp_attr(pp_expression)),
             ppf,
           )
         )

and pp_attr:
  Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_jsx_attribute_t) =
  (pp_expression, ppf, attr) =>
    Fmt.(
      pf(
        ppf,
        "%a%a",
        Expression.((ppf, ((name, _), _)) => Fmt.string(ppf, name)),
        attr,
        ppf =>
          Expression.(
            fun
            | (_, Some((expr, _))) =>
              pf(ppf, "=%a", pp_attr_expr(pp_expression), expr)
            | _ => nop(ppf, ())
          ),
        attr,
      )
    )

and pp_attr_expr:
  Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_expression_t) =
  (pp_expression, ppf) =>
    fun
    | (
        Primitive(_) | Identifier(_) | Group(_) | Closure(_) |
        /* show tags or fragments with no children */
        JSX(Tag(_, _, []) | Fragment([]))
      ) as expr =>
      pp_expression(ppf, expr)
    | expr => Fmt.pf(ppf, "(%a)", pp_expression, expr);
