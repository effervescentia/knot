open Knot.Kore;
open Common;

module NamedImport = {
  type t = (identifier_t, option(identifier_t));

  type node_t = raw_t(t);
};

type export_t =
  | MainExport
  | NamedExport;

type t('typ) =
  | Export(export_t, identifier_t, Declaration.node_t('typ))
  | StdlibImport(list(NamedImport.node_t))
  | Import(
      raw_t(Reference.Namespace.t),
      option(identifier_t),
      list(NamedImport.node_t),
    );

type node_t('typ) = raw_t(t('typ));
