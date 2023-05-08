open Knot.Kore;
open AST;

module Namespace = Reference.Namespace;

type import_spec_t = (
  Namespace.t,
  option(string),
  list((string, option(string))),
);

let format_namespace: Fmt.t(Namespace.t) =
  ppf => ~@Namespace.pp % KString.Plugin.format(ppf);

let format_named_import: Fmt.t((string, option(string))) =
  ppf =>
    fun
    | (id, Some(label)) => Fmt.pf(ppf, "%s as %s", id, label)
    | (id, None) => Fmt.string(ppf, id);

let format_named_import_list: Fmt.t(list((string, option(string)))) =
  (ppf, imports) => Fmt.(destruct(format_named_import, ppf, imports));

let format_main_import: Fmt.t(option(string)) =
  ppf =>
    fun
    | Some(id) => Fmt.string(ppf, id)
    | None => Fmt.nop(ppf, ());

let _format_import_stmt = (pp_targets: Fmt.t('a)): Fmt.t(('a, Namespace.t)) =>
  Fmt.(
    hvbox((ppf, (targets, namespace)) =>
      pf(
        ppf,
        "import %a from %a;",
        pp_targets,
        targets,
        format_namespace,
        namespace,
      )
    )
  );

let format: Fmt.t(import_spec_t) =
  ppf =>
    fun
    | (_, None, []) => Fmt.nop(ppf, ())

    | (namespace, Some(main_import), []) =>
      _format_import_stmt(Fmt.string, ppf, (main_import, namespace))

    | (namespace, None, named_imports) =>
      _format_import_stmt(
        format_named_import_list,
        ppf,
        (named_imports, namespace),
      )

    | (namespace, Some(main_import), named_imports) =>
      _format_import_stmt(
        (ppf, (main, named)) =>
          Fmt.(pf(ppf, "%s, %a", main, format_named_import_list, named)),
        ppf,
        ((main_import, named_imports), namespace),
      );
