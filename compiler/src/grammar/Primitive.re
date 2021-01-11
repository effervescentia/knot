open Kore;

let nil =
  Keyword.nil
  >|= snd
  % (cursor => Block.create(~type_=Type.K_Nil, cursor, AST.nil))
  |> M.lexeme;

let boolean =
  Keyword.true_
  >|= snd
  % (cursor => Block.create(~type_=Type.K_Boolean, cursor, true))
  <|> (
    Keyword.false_
    >|= snd
    % (cursor => Block.create(~type_=Type.K_Boolean, cursor, false))
  )
  >== AST.of_bool
  |> M.lexeme;

let number = Number.parser >|= Block.wrap >== AST.of_num;

let string = M.string >== AST.of_string >@ Type.K_String;

let parser = choice([nil, boolean, number, string]);
