open Core;

let newline = Char('\n');
let tab = Char('\t');
let underscore = Char('_');
let space = Char(' ');
let quote = Char('"');
let dollar_sign = Char('$');

let double_slash = Token("//");
let triple_slash = Token("///");

let whitespace = Either([space, tab, newline]);
