open Knot.Kore;
open AST.Common;

module NamedImport = {
  type t = (identifier_t, option(identifier_t));

  type node_t = raw_t(t);
};
