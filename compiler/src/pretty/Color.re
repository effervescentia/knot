open Kore;

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
