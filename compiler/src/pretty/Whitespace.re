open Kore;

let __indent_spaces = 2;

let indent = ppf => pp_print_break(ppf, 0, __indent_spaces);

let space_or_indent = ppf =>
  pp_print_custom_break(
    ppf,
    ~fits=("", 1, ""),
    ~breaks=("", __indent_spaces, ""),
  );
