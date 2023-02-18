open Knot.Kore;
open AST.Common;

module NamedImport = KImport.Interface.NamedImport;

module ExportKind = {
  type t =
    | Main
    | Named;

  /* methods */

  let to_string =
    fun
    | Main => "Main"
    | Named => "Named";

  let is_main =
    fun
    | Main => true
    | Named => false;

  let fold = (~main, ~named) =>
    fun
    | Main => main()
    | Named => named();
};

type t('decl) =
  | StdlibImport(list(NamedImport.node_t))
  | Import(
      (
        Reference.Namespace.t,
        option(identifier_t),
        list(NamedImport.node_t),
      ),
    )
  | Export((ExportKind.t, identifier_t, 'decl));

type node_t('decl) = raw_t(t('decl));

/* static */

let of_stdlib_import = x => StdlibImport(x);
let of_import = x => Import(x);
let of_export = x => Export(x);

/* methods */

let fold = (~stdlib_import, ~import, ~export) =>
  fun
  | StdlibImport(x) => stdlib_import(x)
  | Import(x) => import(x)
  | Export(x) => export(x);
