open Kore;

type t = {
  trail: formatter => unit,
  hint: (formatter, unit) => unit,
};

let _nop_trail = ppf => nop(ppf, ());
let _trail_sep = (sep, ppf) =>
  pp_print_custom_break(ppf, ~fits=("", 0, ""), ~breaks=(sep, 0, ""));

/* static */

let create = (~trail=_nop_trail, hint: (formatter, unit) => unit): t => {
  trail,
  hint,
};

let of_sep = (~trail=?, sep: string): t =>
  create(~trail=Option.value(trail, ~default=_trail_sep(sep)), (ppf, ()) =>
    pf(ppf, "%s@ ", sep)
  );

let space = create(~trail=_nop_trail, sp);

let trailing_comma = of_sep(",");
let comma = of_sep(~trail=_nop_trail, ",");

let trailing_newline = create(~trail=ppf => cut(ppf, ()), cut);
let newline = create(cut);

let double_newline = create((ppf, ()) => pf(ppf, "@,@,"));

let nop = create(nop);
