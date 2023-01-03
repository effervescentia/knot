/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;
open Common;

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

type t =
  | StdlibImport(list(NamedImport.node_t))
  | Import(
      Reference.Namespace.t,
      option(identifier_t),
      list(NamedImport.node_t),
    )
  | Export(
      ExportKind.t,
      identifier_t,
      Declaration.node_t(Expression.t(Type.t), Type.t),
    );

type node_t = raw_t(t);
