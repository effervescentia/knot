/**
 * This file repackages parser combinators from the OCaml library Opal.
 * The original code can be found here: https://github.com/pyrocat101/opal
 */
open Globals;

let return = Opal.return;
let mzero = Opal.mzero;

let any = Opal.any;
let eof = Opal.eof;

let (>>=) = Opal.(>>=);
let (<|>) = Opal.(<|>);

let (==>) = (x, y) => x >>= (r => y(r) |> return);
let (>>) = Opal.(>>);
let (<<) = Opal.(<<);
let (<~>) = Opal.(<~>);

let satisfy = Opal.satisfy;

let opt = Opal.option;
let optional = Opal.optional;
let between = Opal.between;

let skip_many = Opal.skip_many;

let many = Opal.many;
let many1 = Opal.many1;

let sep_by1 = Opal.sep_by1;
let sep_by = Opal.sep_by;

let chainl1 = Opal.chainl1;
