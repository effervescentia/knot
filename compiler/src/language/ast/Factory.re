/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;

/**
 abstraction on the type of the nodes that makeup an AST
 */
module type ASTParams = {type type_t;};

/**
 constructor for AST modules
 */
module Make = (Params: ASTParams) => {
  include Common;

  /**
   type for AST expression nodes
   */
  type type_t = Params.type_t;

  /**
   container for AST nodes
   */
  type node_t('a) = Node.t('a, type_t);
};
