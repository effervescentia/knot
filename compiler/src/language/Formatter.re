open Kore;
open FormatterUtils;

let __default_margin = 120;

let pp_declaration_list: Fmt.t(list((string, A.raw_declaration_t))) =
  ppf => {
    let rec loop =
      fun
      | [] => Fmt.nop(ppf, ())

      /* do not add newline after the last statement */
      | [decl] =>
        KDeclaration.Plugin.pp(KTypeExpression.Plugin.pp, ppf, decl)

      /* handle constant clustering logic, separate with newlines */
      | [(_, A.Constant(_)) as decl, ...[(_, A.Constant(_)), ..._] as xs] => {
          KDeclaration.Plugin.pp(KTypeExpression.Plugin.pp, ppf, decl);
          Fmt.cut(ppf, ());

          loop(xs);
        }

      /* followed by declarations that are not constants, add a full line break */
      | [decl, ...xs] => {
          KDeclaration.Plugin.pp(KTypeExpression.Plugin.pp, ppf, decl);
          Fmt.cut(ppf, ());
          Fmt.cut(ppf, ());

          loop(xs);
        };

    loop;
  };

let pp_import_list: Fmt.t(list(KImport.Formatter.import_spec_t)) =
  ppf =>
    Fmt.(list(~layout=Vertical, ~sep=Sep.newline, KImport.Plugin.pp, ppf));

let pp_all_imports:
  Fmt.t(
    (
      list(KImport.Formatter.import_spec_t),
      list(KImport.Formatter.import_spec_t),
    ),
  ) =
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
