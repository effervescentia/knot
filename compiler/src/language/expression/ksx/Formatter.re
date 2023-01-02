open Knot.Kore;
open AST;

let rec format: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.ksx_t) =
  (pp_expression, ppf) =>
    fun
    | Tag(_, (name, _), styles, attributes, []) =>
      Fmt.pf(
        ppf,
        "@[<h><%s%a%a@ />@]",
        name,
        format_style_binding_list(pp_expression),
        styles |> List.map(fst),
        format_attribute_list(pp_expression),
        attributes |> List.map(fst),
      )

    | Tag(_, (name, _), styles, attributes, children) =>
      Fmt.(
        pf(
          ppf,
          "@[<h><%s%a%a>@]%a</%s>",
          name,
          format_style_binding_list(pp_expression),
          styles |> List.map(fst),
          format_attribute_list(pp_expression),
          attributes |> List.map(fst),
          block(
            ~layout=Vertical,
            ~sep=Sep.trailing_newline,
            format_child(pp_expression),
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
             format_child(pp_expression),
             ppf,
           )
         )

and format_style_binding_list = pp_expression =>
  Fmt.(list(~sep=Sep.nop, format_style_binding(pp_expression)))

and format_style_binding = (pp_expression, ppf, style) =>
  Fmt.(pf(ppf, "::%a", pp_expression, style))

and format_child:
  Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_ksx_child_t) =
  (pp_expression, ppf) =>
    fun
    | Node(jsx) => jsx |> Fmt.pf(ppf, "%a", format(pp_expression))
    | Text(text) => text |> Fmt.string(ppf)
    | InlineExpression((expr, _)) => Fmt.pf(ppf, "{%a}", pp_expression, expr)

and format_attribute_list:
  Fmt.t(Result.raw_expression_t) => Fmt.t(list(Result.raw_ksx_attribute_t)) =
  (pp_expression, ppf) =>
    fun
    | [] => Fmt.nop(ppf, ())
    | attrs =>
      attrs
      |> Fmt.(
           list(
             ~sep=Sep.nop,
             ppf => pf(ppf, "@ %a", format_attribute(pp_expression)),
             ppf,
           )
         )

and format_attribute:
  Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_ksx_attribute_t) =
  (pp_expression, ppf, attribute) =>
    Fmt.pf(
      ppf,
      "%a%a",
      ppf =>
        fun
        | ((name, _), _) => Fmt.string(ppf, name),
      attribute,
      ppf =>
        fun
        | (_, Some((expr, _))) =>
          Fmt.pf(
            ppf,
            "=%a",
            format_attribute_expression(pp_expression),
            expr,
          )
        | _ => Fmt.nop(ppf, ()),
      attribute,
    )

and format_attribute_expression:
  Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_expression_t) =
  (pp_expression, ppf) =>
    fun
    | (
        Primitive(_) | Identifier(_) | Group(_) | Closure(_) |
        /* show tags or fragments with no children */
        KSX(Tag(_, _, _, _, []) | Fragment([]))
      ) as expression =>
      pp_expression(ppf, expression)
    | expr => Fmt.pf(ppf, "(%a)", pp_expression, expr);
