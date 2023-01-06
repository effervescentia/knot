open Knot.Kore;
open AST.Common;

module ExportKind = {
  type t =
    | Main
    | Named;

  let to_string =
    fun
    | Main => "Main"
    | Named => "Named";

  let is_main =
    fun
    | Main => true
    | Named => false;
};

module NamedImport = {
  type t = (identifier_t, option(identifier_t));

  type node_t = raw_t(t);
};

type t('decl) =
  | StdlibImport(list(NamedImport.node_t))
  | Import(
      Reference.Namespace.t,
      option(identifier_t),
      list(NamedImport.node_t),
    )
  | Export(ExportKind.t, identifier_t, 'decl);

type node_t('decl) = raw_t(t('decl));
