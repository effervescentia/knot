/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;

open Factory.Make({
       type type_t = Type.t;
     });

/**
 a declaration AST node
 */
type declaration_t = node_t(raw_declaration_t)
/**
 supported module declarations
 */
and raw_declaration_t =
  | Constant(expression_t)
  | Enumerated(list((identifier_t, list(node_t(TypeExpression.raw_t)))))
  | Function(list(argument_t), expression_t)
  | View(list(argument_t), list(Node.t(string, Type.t)), expression_t);

/**
 a stdlib import AST node
 */
type named_import_t = untyped_t((identifier_t, option(identifier_t)));

/**
 supported export types
 */
type export_t =
  | Main
  | Named;

/**
 module statement AST node
 */
type module_statement_t = untyped_t(raw_module_statement_t)
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
  | Export(export_t, identifier_t, declaration_t);

/**
 the AST of an entire module
 */
type program_t = list(module_statement_t);
