open Knot.Kore;
open Common;

type t('typ) =
  | Primitive(Primitive.t)
  | Identifier(string)
  | KSX(KSX.node_t(t('typ), 'typ))
  | Style(list(raw_t((identifier_t, node_t('typ)))))
  | Group(node_t('typ))
  | Closure(list(Statement.node_t(t('typ), 'typ)))
  | UnaryOperation(Operator.Unary.t, node_t('typ))
  | BinaryOperation(Operator.Binary.t, node_t('typ), node_t('typ))
  | DotAccess(node_t('typ), identifier_t)
  | BindStyle(KSX.tag_t, node_t('typ), node_t('typ))
  | FunctionCall(node_t('typ), list(node_t('typ)))

and node_t('typ) = Node.t(t('typ), 'typ);
