open Parsing;
open Knot.Token;

let space = one_of([Space, Tab, Newline]);
let spaces = skip_many(space);

let lexeme = x => spaces >> x;