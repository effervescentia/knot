open Kore;

let nil = AST.nil <$ Keyword.nil |> M.lexeme;

let boolean =
  true
  <$ Keyword.true_
  <|> (false <$ Keyword.false_)
  >|= AST.of_bool
  |> M.lexeme;

let integer =
  many1(M.digit)
  >|= Char.join
  % fst
  % Int64.of_string
  % AST.of_int
  |> M.lexeme;

let float =
  many1(M.digit)
  >>= Char.join
  % (
    is =>
      Character.period
      >> many1(M.digit)
      >|= Char.join
      % (fs => Print.fmt("%s.%s", fst(is), fst(fs)))
      % Float.of_string
      % AST.of_float
  )
  |> M.lexeme;

let number = choice([integer, float]) >|= AST.of_num;

let string = M.string >|= fst % AST.of_string;

let parser = choice([nil, boolean, number, string]);
