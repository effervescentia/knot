/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;

open Factory.Make({
       type type_t = Type.t;
     });

/**
 a stdlib import AST node
 */
type named_import_t = raw_t((identifier_t, option(identifier_t)));

/**
 supported export types
 */
type export_kind_t =
  | Main
  | Named;

/**
 module statement AST node
 */
type module_statement_t = raw_t(raw_module_statement_t)
/**
 supported top-level module statements
 */
and raw_module_statement_t =
  | StdlibImport(list(named_import_t))
  | Import(
      Reference.Namespace.t,
      option(identifier_t),
      list(named_import_t),
    )
  | Export(
      export_kind_t,
      identifier_t,
      Declaration.node_t(Expression.t(Type.t), Type.t),
    );

/**
 the AST of an entire module
 */
type program_t = list(module_statement_t);
