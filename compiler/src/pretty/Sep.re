open Kore;

type t = {
  trail: formatter => unit,
  hint: (formatter, unit) => unit,
};

module Trail = {
  let of_sep = (sep, ppf) =>
    pp_print_custom_break(ppf, ~fits=("", 0, ""), ~breaks=(sep, 0, ""));

  /* pre-defined */

  let nop = ppf => nop(ppf, ());

  let space = ppf => sp(ppf, ());

  let newline = ppf => cut(ppf, ());

  let space_or_comma = ppf =>
    pp_print_custom_break(ppf, ~fits=("", 1, ""), ~breaks=(",", 0, ""));
};

/* static */

let create = (~trail=Trail.nop, hint: (formatter, unit) => unit): t => {
  trail,
  hint,
};

let of_sep = (~trail=?, sep: string): t =>
  create(~trail=Option.value(trail, ~default=Trail.of_sep(sep)), (ppf, ()) =>
    pf(ppf, "%s@ ", sep)
  );

/* pre-defined */

let nop = create(nop);

let space = create(~trail=Trail.nop, sp);

let trailing_comma = of_sep(",");
let comma = of_sep(~trail=Trail.nop, ",");

let trailing_newline = create(~trail=ppf => cut(ppf, ()), cut);
let newline = create(cut);

let _double_newline_hint = (ppf, ()) => pf(ppf, "@,@,");
let double_newline = create(_double_newline_hint);
let trailing_double_newline =
  create(~trail=ppf => cut(ppf, ()), _double_newline_hint);
