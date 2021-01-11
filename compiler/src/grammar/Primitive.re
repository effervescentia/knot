open Kore;

let nil = AST.nil <$ Keyword.nil |> M.lexeme;

let boolean =
  true
  <$ Keyword.true_
  <|> (false <$ Keyword.false_)
  >|= AST.of_bool
  |> M.lexeme;

let number = Number.parser >|= AST.of_num;

let string = M.string >|= fst % AST.of_string;

let parser = choice([nil, boolean, number, string]);
