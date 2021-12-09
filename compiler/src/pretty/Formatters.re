include Fmt;
include Color;
include Whitespace;
include Container;
include XML;

module Sep = Sep;

let uchar = (ppf, uc) => {
  let ui = Uchar.to_int(uc);
  if (ui > 31 && ui < 127 || ui == 9 || ui == 10 || ui == 13) {
    string(ppf, Char.escaped(Uchar.to_char(uc)));
  } else {
    pf(ppf, "\\u{%x}", ui);
  };
};
