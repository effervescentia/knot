open Knot.Kore;
open AST.Common;

type t('expr, 'typ) =
  | Variable(identifier_t, Node.t('expr, 'typ))
  | Effect(Node.t('expr, 'typ));

type node_t('expr, 'typ) = Node.t(t('expr, 'typ), 'typ);

/* plugin types */

module Plugin = {
  include AST.Framework.AnalyzePlugin.Make({
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) =
      AST.Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr);
    type value_t('expr, 'typ) = t('expr, 'typ);
  });
  include AST.Framework.FormatPlugin.Make({
    type format_arg_t('expr, 'typ) = unit;
    type value_t('expr, 'typ) = t('expr, 'typ);
  });
  include AST.Framework.DebugPlugin.Make({
    type debug_arg_t('expr, 'typ) = (
      AST.Framework.debug_node_t('expr, 'typ),
      'typ => string,
    );
    type value_t('expr, 'typ) = node_t('expr, 'typ);
  });
};

/* static */

let of_variable = ((name, expression)) => Variable(name, expression);
let of_effect = expression => Effect(expression);

/* methods */

let fold = (~variable, ~effect) =>
  fun
  | Variable(name, expression) => (name, expression) |> variable
  | Effect(expression) => expression |> effect;
