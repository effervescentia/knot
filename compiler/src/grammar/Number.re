open Kore;

let integer =
  many1(M.digit) >|= Char.join >== Int64.of_string >== AST.of_int |> M.lexeme;

let float =
  M.binary_op(
    many1(M.digit) >|= Char.join,
    Character.period,
    many1(M.digit) >|= Char.join,
  )
  >|= (((x, y)) => Block.join(~combine=Print.fmt("%s.%s"), x, y))
  >== Float.of_string
  >== AST.of_float
  |> M.lexeme;

let parser = choice([float, integer]);
