/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;
open Common;

type t('expr, 'typ) =
  | Constant(Node.t('expr, 'typ))
  | Enumerated(
      list((identifier_t, list(Node.t(TypeExpression.raw_t, 'typ)))),
    )
  | Function(
      list(Expression.Parameter.node_t('expr, 'typ)),
      Node.t('expr, 'typ),
    )
  | View(
      list(Expression.Parameter.node_t('expr, 'typ)),
      list(Node.t(string, 'typ)),
      Node.t('expr, 'typ),
    );

type node_t('expr, 'typ) = Node.t(t('expr, 'typ), 'typ);
