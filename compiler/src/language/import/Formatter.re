open Knot.Kore;

type import_spec_t = (
  Reference.Namespace.t,
  option(string),
  list((string, option(AST.identifier_t))),
);

let pp_namespace: Fmt.t(Reference.Namespace.t) =
  ppf => ~@Reference.Namespace.pp % KString.Plugin.pp(ppf);

let pp_named_import: Fmt.t((string, option(AST.identifier_t))) =
  ppf =>
    fun
    | (id, Some((label, _))) => Fmt.pf(ppf, "%s as %s", id, label)
    | (id, None) => Fmt.string(ppf, id);

let pp_named_import_list: Fmt.t(list((string, option(AST.identifier_t)))) =
  (ppf, imports) => Fmt.(destruct(pp_named_import, ppf, imports));

let pp_main_import: Fmt.t(option(string)) =
  ppf =>
    fun
    | Some(id) => Fmt.string(ppf, id)
    | None => Fmt.nop(ppf, ());

let _pp_import_stmt =
    (pp_targets: Fmt.t('a)): Fmt.t(('a, Reference.Namespace.t)) =>
  Fmt.(
    hvbox((ppf, (targets, namespace)) =>
      pf(
        ppf,
        "import %a from %a;",
        pp_targets,
        targets,
        pp_namespace,
        namespace,
      )
    )
  );

let pp_import: Fmt.t(import_spec_t) =
  ppf =>
    fun
    | (namespace, None, []) => Fmt.nop(ppf, ())

    | (namespace, Some(main_import), []) =>
      _pp_import_stmt(Fmt.string, ppf, (main_import, namespace))

    | (namespace, None, named_imports) =>
      _pp_import_stmt(pp_named_import_list, ppf, (named_imports, namespace))

    | (namespace, Some(main_import), named_imports) =>
      _pp_import_stmt(
        (ppf, (main, named)) =>
          Fmt.(pf(ppf, "%s, %a", main, pp_named_import_list, named)),
        ppf,
        ((main_import, named_imports), namespace),
      );
