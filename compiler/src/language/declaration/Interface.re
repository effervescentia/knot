open Knot.Kore;
open AST.Common;

type t('expr, 'typ) =
  | Constant(Node.t('expr, 'typ))
  | Enumerated(
      list(
        (identifier_t, list(Node.t(KTypeExpression.Interface.t, 'typ))),
      ),
    )
  | Function(
      list(KLambda.Interface.Parameter.node_t('expr, 'typ)),
      Node.t('expr, 'typ),
    )
  | View(
      list(KLambda.Interface.Parameter.node_t('expr, 'typ)),
      list(Node.t(string, 'typ)),
      Node.t('expr, 'typ),
    );

type node_t('expr, 'typ) = Node.t(t('expr, 'typ), 'typ);

let fold = (~constant, ~enumerated, ~function_, ~view) =>
  fun
  | Constant(expr) => expr |> constant
  | Enumerated(variants) => variants |> enumerated
  | Function(args, expr) => (args, expr) |> function_
  | View(props, mixins, expr) => (props, mixins, expr) |> view;
