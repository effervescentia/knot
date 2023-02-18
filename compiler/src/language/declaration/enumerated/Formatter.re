open Knot.Kore;
open AST;

let format_parameter_list =
  Fmt.(
    (ppf, parameters) =>
      List.is_empty(parameters)
        ? ()
        : pf(ppf, "(%a)", list(KTypeExpression.Plugin.format), parameters)
  );

let format_variant_list = variants =>
  Fmt.(
    block(
      ~layout=Vertical,
      ~sep=Sep.space,
      (ppf, ((variant_name, _), parameters)) =>
        pf(
          ppf,
          "@[<h>| %s%a@]",
          variant_name,
          format_parameter_list,
          parameters |> List.map(fst),
        ),
      variants,
    )
  );

let format: Interface.Plugin.format_t('typ) =
  ppf =>
    fun
    | (name, []) => Fmt.(pf(ppf, "enum %s = | ;", name))
    | (name, variants) =>
      Fmt.(
        pf(ppf, "@[<v>enum %s =%a;@]", name, format_variant_list, variants)
      );
