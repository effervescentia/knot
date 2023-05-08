open Knot.Kore;

module ObjectEntry = {
  type t('t) =
    | Required((AST.Common.identifier_t, 't))
    | Optional((AST.Common.identifier_t, 't))
    | Spread('t);

  type node_t('t) = AST.Common.raw_t(t('t));

  /* static */

  let of_required = x => Required(x);
  let of_optional = x => Optional(x);
  let of_spread = x => Spread(x);

  /* methods */

  let fold = (~required, ~optional, ~spread) =>
    fun
    | Required(x) => required(x)
    | Optional(x) => optional(x)
    | Spread(x) => spread(x);
};

type t =
  | Nil
  | Boolean
  | Integer
  | Float
  | String
  | Element
  | Style
  | Identifier(AST.Common.identifier_t)
  | Group(node_t)
  | List(node_t)
  | Object(list(ObjectEntry.node_t(node_t)))
  | Function((list(node_t), node_t))
  | DotAccess((node_t, AST.Common.identifier_t))
  | View((node_t, node_t))

and node_t = AST.Common.raw_t(t);

/* static */

let of_identifier = x => Identifier(x);
let of_group = x => Group(x);
let of_list = x => List(x);
let of_object = x => Object(x);
let of_function = x => Function(x);
let of_dot_access = x => DotAccess(x);
let of_view = x => View(x);

/* methods */

let fold =
    (
      ~nil,
      ~boolean,
      ~integer,
      ~float,
      ~string,
      ~element,
      ~style,
      ~identifier,
      ~group,
      ~list,
      ~object_,
      ~function_,
      ~dot_access,
      ~view,
    ) =>
  fun
  | Nil => nil()
  | Boolean => boolean()
  | Integer => integer()
  | Float => float()
  | String => string()
  | Element => element()
  | Style => style()
  | Identifier(x) => identifier(x)
  | Group(x) => group(x)
  | List(x) => list(x)
  | Object(x) => object_(x)
  | Function(x) => function_(x)
  | DotAccess(x) => dot_access(x)
  | View(x) => view(x);
