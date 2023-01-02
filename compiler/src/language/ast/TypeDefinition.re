/**
 Type definition module Abstract Syntax Tree.
 */
open Knot.Kore;
open Common;

module TypeExpression = TypeExpression;

type variant_t = (untyped_t(string), list(TypeExpression.t));

type decorator_t('a) =
  untyped_t((Node.t(string, 'a), list(Node.t(primitive_t, 'a))));

type module_statement_t = untyped_t(raw_module_statement_t)

and raw_module_statement_t =
  | Declaration(untyped_t(string), TypeExpression.t)
  | Type(untyped_t(string), TypeExpression.t)
  | Enumerated(untyped_t(string), list(variant_t));

type module_t = untyped_t(raw_module_t)

and raw_module_t =
  | Decorator(
      untyped_t(string),
      list(TypeExpression.t),
      Type.DecoratorTarget.t,
    )
  | Module(
      untyped_t(string),
      list(module_statement_t),
      list(decorator_t(Type.t)),
    );

type t = list(module_t);

/* tag helpers */

let of_decorator = ((id, args, target)) => Decorator(id, args, target);
let of_exportaration = ((id, type_)) => Declaration(id, type_);
let of_type = ((id, type_)) => Type(id, type_);
let of_enum = ((id, variants)) => Enumerated(id, variants);
let of_module = ((id, stmts, decorators)) => Module(id, stmts, decorators);
