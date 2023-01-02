open Knot.Kore;
open Common;

module Child = {
  type t('expr, 'ksx) =
    | Text(string)
    | Inline('expr)
    | KSX('ksx);

  type node_t('expr, 'ksx) = raw_t(t('expr, 'ksx));
};

type tag_t =
  | View
  | Element;

type t('expr, 'typ) =
  | Tag(
      tag_t,
      Node.t(string, 'typ),
      list('expr),
      list(raw_t((identifier_t, option('expr)))),
      list(Child.node_t('expr, t('expr, 'typ))),
    )
  | Fragment(Child.node_t('expr, t('expr, 'typ)))

and node_t('expr, 'typ) = raw_t(t('expr, 'typ));
