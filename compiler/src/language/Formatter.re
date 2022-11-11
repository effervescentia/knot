open Knot.Kore;
open FormatterUtils;
open AST;

let __default_margin = 120;

let pp_declaration_list: Fmt.t(list((string, Module.raw_declaration_t))) =
  ppf => {
    let rec loop =
      Module.(
        fun
        | [] => Fmt.nop(ppf, ())

        /* do not add newline after the last statement */
        | [decl] => KDeclaration.Plugin.pp(ppf, decl)

        /* handle constant clustering logic, separate with newlines */
        | [(_, Constant(_)) as decl, ...[(_, Constant(_)), ..._] as xs] => {
            KDeclaration.Plugin.pp(ppf, decl);
            Fmt.cut(ppf, ());

            loop(xs);
          }

        /* followed by declarations that are not constants, add a full line break */
        | [decl, ...xs] => {
            KDeclaration.Plugin.pp(ppf, decl);
            Fmt.cut(ppf, ());
            Fmt.cut(ppf, ());

            loop(xs);
          }
      );

    loop;
  };

let pp_import_list: Fmt.t(list(KImport.Formatter.import_spec_t)) =
  ppf =>
    Fmt.(list(~layout=Vertical, ~sep=Sep.newline, KImport.Plugin.pp, ppf));

let pp_standard_import: Fmt.t(list((string, option(string)))) =
  ppf => Fmt.(pf(ppf, "import %a;", KImport.Formatter.pp_named_import_list));

let pp_all_imports:
  Fmt.t(
    (
      list((string, option(string))),
      list(KImport.Formatter.import_spec_t),
      list(KImport.Formatter.import_spec_t),
    ),
  ) =
  ppf =>
    fun
    | ([], [], []) => Fmt.nop(ppf, ())

    | (standard_import, [], []) => pp_standard_import(ppf, standard_import)

    | ([], external_imports, internal_imports) =>
      switch (external_imports, internal_imports) {
      | (only_imports, [])
      | ([], only_imports) => pp_import_list(ppf, only_imports)

      | _ =>
        [external_imports, internal_imports]
        |> Fmt.(
             list(
               ~layout=Vertical,
               ~sep=Sep.double_newline,
               pp_import_list,
               ppf,
             )
           )
      }

    | (standard_import, external_imports, internal_imports) =>
      Fmt.(
        pf(
          ppf,
          "%a@,@,%a",
          pp_standard_import,
          standard_import,
          list(~layout=Vertical, ~sep=Sep.double_newline, pp_import_list),
          [external_imports, internal_imports],
        )
      );

let format = (~margin=__default_margin): Fmt.t(Module.program_t) =>
  (ppf, program) => {
    let orig_margin = Format.get_margin();
    Format.set_margin(margin);

    program
    |> Tuple.split2(extract_imports, extract_declarations)
    |> Fmt.(
         page(
           (ppf, (imports, declarations)) =>
             switch (imports, declarations) {
             | (([], [], []), []) => Fmt.nop(ppf, ())

             | (_, []) => pp_all_imports(ppf, imports)

             | (([], [], []), _) => pp_declaration_list(ppf, declarations)

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
