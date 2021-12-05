open Infix;

module Fmt = {
  include Fmt;
  include Color;

  let __indent_spaces = 2;

  type layout_t =
    | Vertical
    | Horizontal;

  module Sep = {
    type t = {
      trail: Format.formatter => unit,
      hint: (Format.formatter, unit) => unit,
    };

    let _nop_trail = ppf => nop(ppf, ());
    let _trail_sep = (sep, ppf) =>
      Format.pp_print_custom_break(
        ppf,
        ~fits=("", 0, ""),
        ~breaks=(sep, 0, ""),
      );

    /* static */

    let create =
        (~trail=_nop_trail, hint: (Format.formatter, unit) => unit): t => {
      trail,
      hint,
    };

    let of_sep = (~trail=?, sep: string): t =>
      create(~trail=trail |?: _trail_sep(sep), (ppf, ()) =>
        pf(ppf, "%s@ ", sep)
      );

    let trailing_comma = of_sep(",");
    let comma = of_sep(~trail=_nop_trail, ",");

    let trailing_newline = create(~trail=ppf => cut(ppf, ()), cut);
    let newline = create(cut);

    let double_newline = create((ppf, ()) => pf(ppf, "@,@,"));

    let nop = create(nop);
  };

  /* value formatters */

  let uchar = (ppf, uc) => {
    let ui = Uchar.to_int(uc);
    if (ui > 31 && ui < 127 || ui == 9 || ui == 10 || ui == 13) {
      string(ppf, Char.escaped(Uchar.to_char(uc)));
    } else {
      pf(ppf, "\\u{%x}", ui);
    };
  };

  /* layout formatters */

  let root = (pp_value: t('a)): t('a) => vbox(pp_value);

  let page = (pp_value: t('a)): t('a) =>
    ppf => pf(ppf, "%a@.", root(pp_value));

  let indent = ppf => Format.pp_print_break(ppf, 0, __indent_spaces);

  let indent_or_space = ppf =>
    Format.pp_print_custom_break(
      ppf,
      ~fits=("", 1, ""),
      ~breaks=("", __indent_spaces, ""),
    );

  let space_or_cop = ppf =>
    Format.pp_print_custom_break(
      ppf,
      ~fits=("", 1, ""),
      ~breaks=("", __indent_spaces, ""),
    );

  /*
   print an indented value
   */
  let indented = (pp_value: t('a)): t('a) =>
    ppf => pf(ppf, "%t%a", indent, pp_value);

  let attribute = (pp_key: t('a), pp_value: t('b)): t(('a, 'b)) =>
    (ppf, (key, value)) => pf(ppf, "%a: %a", pp_key, key, pp_value, value);

  /* container formatters */

  let _list_box =
    fun
    | Vertical => vbox
    | Horizontal => hvbox;

  /**
   print a list of values divided by separators and spaces
   automatically breaks each item onto separate lines for long lists
   set `~trail=true` to show a trailing seprator for long lists
   */
  let list =
      (~layout=Horizontal, ~sep=Sep.trailing_comma, pp_value: t('a))
      : t(list('a)) =>
    ppf =>
      fun
      /* do not add surrounding box when empty */
      | [] => nop(ppf, ())

      | xs => {
          let rec print = ppf => (
            fun
            | [] => nop(ppf, ())

            | [x] => pp_value(ppf, x)

            | [x, ...xs] =>
              pf(ppf, "%a%a%a", pp_value, x, sep.hint, (), print, xs)
          );

          pf(ppf, "%a%t", _list_box(layout, print), xs, sep.trail);
        };

  /*
   print an indented list of values divided by separators
   */
  let block =
      (~layout=Horizontal, ~sep=Sep.trailing_comma, pp_value: t('a))
      : t(list('a)) =>
    ppf =>
      fun
      | [] => nop(ppf, ())

      | xs => indented(list(~layout, ~sep, pp_value), ppf, xs);

  /*
   print a list of values divided by separators
   and bookended by open and close symbols
   */
  let collection =
      (
        ~layout=Horizontal,
        ~sep=Sep.trailing_comma,
        pp_open: t(unit),
        pp_close: t(unit),
        pp_value: t('a),
      )
      : t(list('a)) =>
    (ppf, xs) =>
      pf(
        ppf,
        "%a%a%a",
        pp_open,
        (),
        block(~layout, ~sep, pp_value),
        xs,
        pp_close,
        (),
      );

  let array = (pp_value: t('a)): t(list('a)) =>
    box(collection(any("["), any("]"), pp_value));

  let tuple = (pp_value: t('a)): t(list('a)) =>
    box(collection(any("("), any(")"), pp_value));

  let closure = (pp_value: t('a)): t(list('a)) =>
    ppf =>
      fun
      /* do not allow this space to be broken */
      | [] => string(ppf, "{ }")

      | xs =>
        collection(
          ~layout=Vertical,
          ~sep=Sep.trailing_newline,
          any("{"),
          any("}"),
          pp_value,
          ppf,
          xs,
        );

  let record = (pp_key: t('a), pp_value: t('b)): t(list(('a, 'b))) =>
    closure(attribute(pp_key, pp_value));

  let entity = (pp_sig: t('a), pp_entry: t('b)): t(('a, list('b))) =>
    (ppf, (sig_, entries)) =>
      pf(ppf, "@[<hv>%a@ @]%a", pp_sig, sig_, closure(pp_entry), entries);

  let struct_ =
      (pp_key: t('a), pp_value: t('b)): t((string, list(('a, 'b)))) =>
    (ppf, (name, attrs)) =>
      entity(string, attribute(pp_key, pp_value), ppf, (name, attrs));

  let _space_or_indent = ppf =>
    Format.pp_print_custom_break(
      ppf,
      ~fits=("", 1, ""),
      ~breaks=("", __indent_spaces, ""),
    );
  let _space_or_comma = ppf =>
    Format.pp_print_custom_break(
      ppf,
      ~fits=("", 1, ""),
      ~breaks=(",", 0, ""),
    );
  let _destruct_comma_sep = Sep.of_sep(~trail=_space_or_comma, ",");

  let destruct = (pp_entry, ppf, entries) =>
    pf(
      ppf,
      "{%t%a}",
      _space_or_indent,
      list(~sep=_destruct_comma_sep, pp_entry),
      entries,
    );
};
