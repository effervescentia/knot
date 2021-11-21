module ANSI = ANSITerminal;

module Fmt = {
  include Fmt;

  let color = ref(false);

  let _ansi_sprintf = (x, s) => color^ ? ANSI.sprintf(x, "%s", s) : s;
  let _ansi_code_sprintf = (x, s) =>
    color^ ? str("\027[%dm%s\027[0m", x, s) : s;

  let ansi = (styles: list(ANSI.style), pp_value: t('a)): t('a) =>
    (ppf, value) =>
      pf(ppf, "%s", _ansi_sprintf(styles, str("%a", pp_value, value)));

  let ansi_code = (code: int, pp_value: t('a)): t('a) =>
    (ppf, value) =>
      pf(ppf, "%s", _ansi_code_sprintf(code, str("%a", pp_value, value)));

  let bold = pp => ansi([ANSI.Bold], pp);
  let bold_str = bold(string);

  let red = pp => ansi([ANSI.red], pp);
  let red_str = red(string);

  let green = pp => ansi([ANSI.green], pp);
  let green_str = green(string);

  let cyan = pp => ansi([ANSI.cyan], pp);
  let cyan_str = cyan(string);

  let yellow = pp => ansi([ANSI.yellow], pp);
  let yellow_str = yellow(string);

  let grey = pp => ansi_code(90, pp);
  let grey_str = grey(string);

  let good = pp => pp |> green |> bold;
  let good_str = good(string);

  let bad = pp => pp |> red |> bold;
  let bad_str = bad(string);

  let warn = pp => pp |> yellow |> bold;
  let warn_str = warn(string);

  let uchar = (ppf, uc) => {
    let ui = Uchar.to_int(uc);
    if (ui > 31 && ui < 127 || ui == 9 || ui == 10 || ui == 13) {
      string(ppf, Char.escaped(Uchar.to_char(uc)));
    } else {
      pf(ppf, "\\u{%x}", ui);
    };
  };

  let attr = (pp_key: t('a), pp_value: t('b)): t(('a, 'b)) =>
    (ppf, (key, value)) => pf(ppf, "%a: %a", pp_key, key, pp_value, value);

  let root = (~indent=0, pp_value: t('a)): t('a) =>
    ppf => vbox(~indent, pp_value, ppf);

  let block = (pp_value: t('a)): t('a) =>
    ppf => pf(ppf, "@;<0 2>@[<v 0>%a@]@,", pp_value);

  let arguments = (pp_value: t('a)): t(list('a)) =>
    ppf =>
      fun
      | [] => pf(ppf, "()")
      | xs =>
        pf(
          ppf,
          "@[<hv 0>(%a)@]",
          list(~sep=(ppf, ()) => pf(ppf, ",@ "), pp_value),
          xs,
        );

  let closure = (pp_value: t('a)): t(list('a)) =>
    ppf =>
      fun
      | [] => pf(ppf, "{ }")
      | xs => pf(ppf, "{%a}", block(list(~sep=cut, pp_value)), xs);

  let entity = (pp_sig: t('a), pp_entry: t('b)): t(('a, list('b))) =>
    (ppf, (sig_, entries)) =>
      pf(ppf, "@[<hv 0>%a@ @]%a", pp_sig, sig_, closure(pp_entry), entries);

  let struct_ =
      (name: string, pp_key: t('a), pp_value: t('b)): t(list(('a, 'b))) =>
    (ppf, attrs) =>
      entity(string, attr(pp_key, pp_value), ppf, (name, attrs));
};
