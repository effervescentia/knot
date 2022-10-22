open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

let assign = (id, x) => Matchers.binary_op(id, Symbol.assign, x);
