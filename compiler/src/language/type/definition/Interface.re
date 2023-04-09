open Knot.Kore;
open AST.Common;

type decorator_t =
  raw_t(KDecorator.Interface.t(KPrimitive.Interface.t, AST.Type.t));

type t =
  | Decorator(
      (
        identifier_t,
        list(KTypeExpression.Interface.node_t),
        AST.Type.DecoratorTarget.t,
      ),
    )
  | Module(
      (
        identifier_t,
        list(KTypeStatement.Interface.node_t),
        list(decorator_t),
      ),
    );

type node_t = raw_t(t);

/* static */

let of_decorator = x => Decorator(x);
let of_module = x => Module(x);

/* methods */

let fold = (~decorator, ~module_) =>
  fun
  | Decorator(x) => decorator(x)
  | Module(x) => module_(x);
