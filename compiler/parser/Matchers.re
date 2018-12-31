open Core;
open Parsing;
open Knot.Token;

let kwd = (x, input) => (exactly(Keyword(x)) |> lexeme)(input);

let import = kwd(Import)
and from = kwd(From);

let semicolon = exactly(Semicolon) |> lexeme;

let identifier =
  (
    fun
    | LazyStream.Cons(Identifier(s), next_in) =>
      Some((s, Lazy.force(next_in)))
    | _ => None
  )
  |> lexeme;

let string =
  (
    fun
    | LazyStream.Cons(String(s), next_in) => Some((s, Lazy.force(next_in)))
    | _ => None
  )
  |> lexeme;