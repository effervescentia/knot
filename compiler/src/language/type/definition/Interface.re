open Knot.Kore;
open AST.Common;

type decorator_t('a) =
  raw_t((Node.t(string, 'a), list(Node.t(KPrimitive.Interface.t, 'a))));

type t =
  | Decorator(
      identifier_t,
      list(KTypeExpression.Interface.node_t),
      AST.Type.DecoratorTarget.t,
    )
  | Module(
      identifier_t,
      list(KTypeStatement.Interface.node_t),
      list(decorator_t(AST.Type.t)),
    );

type node_t = raw_t(t);

// type t = list(module_t);

// /* tag helpers */

// let of_decorator = ((id, args, target)) => Decorator(id, args, target);
// let of_exportaration = ((id, type_)) => Declaration(id, type_);
// let of_type = ((id, type_)) => Type(id, type_);
// let of_enum = ((id, variants)) => Enumerated(id, variants);
// let of_module = ((id, stmts, decorators)) => Module(id, stmts, decorators);
