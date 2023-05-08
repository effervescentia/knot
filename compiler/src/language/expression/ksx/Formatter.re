open Knot.Kore;

let rec format: Interface.Plugin.format_t('expr, 'typ) =
  (needs_wrapper, pp_expression, ppf) =>
    Interface.(
      fun
      | Tag(_, (name, _), styles, attributes, []) =>
        Fmt.pf(
          ppf,
          "@[<h><%s%a%a@ />@]",
          name,
          format_style_binding_list(pp_expression),
          styles |> List.map(fst),
          format_attribute_list(needs_wrapper, pp_expression),
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
            format_attribute_list(needs_wrapper, pp_expression),
            attributes |> List.map(fst),
            block(
              ~layout=Vertical,
              ~sep=Sep.trailing_newline,
              format_child(needs_wrapper, pp_expression),
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
               format_child(needs_wrapper, pp_expression),
               ppf,
             )
           )
    )

and format_style_binding_list = pp_expression =>
  Fmt.(list(~sep=Sep.nop, format_style_binding(pp_expression)))

and format_style_binding = (pp_expression, ppf, style) =>
  Fmt.(pf(ppf, "::%a", pp_expression, style))

and format_child = (needs_wrapper, pp_expression: Fmt.t('expr), ppf) =>
  Interface.Child.(
    fun
    | Node(jsx) =>
      jsx |> Fmt.pf(ppf, "%a", format(needs_wrapper, pp_expression))
    | Text(text) => text |> Fmt.string(ppf)
    | InlineExpression((expr, _)) => Fmt.pf(ppf, "{%a}", pp_expression, expr)
  )

and format_attribute_list = (needs_wrapper, pp_expression: Fmt.t('expr), ppf) =>
  fun
  | [] => Fmt.nop(ppf, ())
  | attrs =>
    attrs
    |> Fmt.(
         list(
           ~sep=Sep.nop,
           ppf =>
             pf(ppf, "@ %a", format_attribute(needs_wrapper, pp_expression)),
           ppf,
         )
       )

and format_attribute =
    (
      needs_wrapper,
      pp_expression: Fmt.t('expr),
      ppf,
      attribute: Interface.Attribute.t('expr, 'typ),
    ) =>
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
          format_attribute_expression(needs_wrapper, pp_expression),
          expr,
        )
      | _ => Fmt.nop(ppf, ()),
    attribute,
  )

and format_attribute_expression =
    (needs_wrapper, pp_expression: Fmt.t('expr), ppf, expression) =>
  if (needs_wrapper(expression)) {
    Fmt.pf(ppf, "(%a)", pp_expression, expression);
  } else {
    pp_expression(ppf, expression);
  };
