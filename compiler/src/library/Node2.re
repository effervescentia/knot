open Infix;

type t('a, 'b) = ('a, ('b, Range.t));

/* static */

let typed = (value: 'a, type_: 'b, range: Range.t): t('a, 'b) => (
  value,
  (type_, range),
);

let untyped = (value: 'a, range: Range.t): t('a, unit) => (
  value,
  ((), range),
);

/* methods */

let get_type: t('a, 'b) => 'b = node => node |> snd |> fst;
let get_range: t('a, 'b) => Range.t = node => node |> snd |> snd;

let map = (f: 'a => 'c, node: t('a, 'b)): t('c, 'b) =>
  node |> Tuple.map_fst2(f);

let map_meta = (f: 'b => 'c, node: t('a, 'b)): t('a, 'c) =>
  node |> Tuple.map_snd2(Tuple.map_fst2(f));

let map_range = (f: Range.t => Range.t, node: t('a, 'b)): t('a, 'b) =>
  node |> Tuple.map_snd2(Tuple.map_snd2(f));

let add_type = (type_: 'a, node: t('b, 'c)): t('b, 'a) =>
  node |> map_meta(_ => type_);

let drop_type = (node: t('a, 'b)): t('a, unit) => node |> map_meta(_ => ());

let join_ranges = (lhs: t('a, 'b), rhs: t('c, 'd)) =>
  Range.join(get_range(lhs), get_range(rhs));

let wrap = (f: t('a, 'b) => 'c, (_, meta) as node: t('a, 'b)): t('c, 'b) => (
  f(node),
  meta,
);

/* pretty printing */

let pp = (pp_value: Fmt.t('a), pp_meta: Fmt.t('b)): Fmt.t(t('a, 'b)) =>
  (ppf, node: t('a, 'b)) =>
    Fmt.pf(
      ppf,
      "%a (%a) %@ %a",
      pp_value,
      fst(node),
      pp_meta,
      get_type(node),
      Range.pp,
      get_range(node),
    );

let pp_untyped = (pp_value: Fmt.t('a)): Fmt.t(t('a, 'b)) =>
  (ppf, node: t('a, 'b)) =>
    Fmt.pf(ppf, "%a %@ %a", pp_value, fst(node), Range.pp, get_range(node));
